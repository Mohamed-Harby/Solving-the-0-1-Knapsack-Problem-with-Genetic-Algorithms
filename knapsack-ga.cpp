#include<bits/stdc++.h>
#define MAX_GENERATIONS 1000
using namespace std;

int knapsack_size;

class Chromosome {
private:
    vector<bool> binary;
    int weight;
    int value;
    int fitness;
    void calc_weight(vector<int> &weight) {
        this->weight = 0;
        for (int i = 0; i < binary.size(); ++i) {
            if (binary[i]) this->weight += weight[i];
        }
    }
    void calc_value(vector<int> &value) {
        this->value = 0;
        for (int i = 0; i < binary.size(); ++i) {
            if (binary[i]) this->value += value[i];
        }
    }
    void calc_fitness() {
        if (this->weight <= knapsack_size) {
            fitness = this->value;
        } else {
            fitness = 0;
        }
    }
public:
    Chromosome(vector<bool> &binary_representation, vector<int> &value, vector<int> &weight) {
        binary = binary_representation;
        calc_value(value);
        calc_weight(weight);
        calc_fitness();
    }
    int get_value() {
        return this->value;
    }
    int get_weight() {
        return this->weight;
    }
    int get_fitness() {
        return this->fitness;
    }
    int get_length() {
        return this->binary.size();
    }
    vector<bool> get_binary() {
        return this->binary;
    }
    bool operator<(Chromosome &temp) {
        return (this->fitness < temp.get_fitness());
    }
    void mutate(vector<int> &value, vector<int> &weight) {
        int index = rand() % binary.size();
        binary[index] = !binary[index];
        calc_value(value);
        calc_weight(weight);
        calc_fitness();
    }
    //------------------------------------

    void to_have_value(vector<int> &value, vector<int> &weight) { // when fitness =0  to have a value^^
        vector<bool> new_bin = this->binary;
        int index = find(new_bin.begin(), new_bin.end(), true) - new_bin.begin();
        if (index == new_bin.size()) return;
        new_bin[index] = 0;
        Chromosome modified(new_bin, value, weight);
        (*this).equalize(modified);
        // base case:
        if (this->fitness != 0) return;
        this->to_have_value(value, weight);
    }

    void equalize(Chromosome &old_chromosome) {
        this->weight = old_chromosome.get_weight();
        this->value = old_chromosome.get_value();
        this->fitness = old_chromosome.get_fitness();
        this->binary = old_chromosome.get_binary();

    }

    void print() {
        cout << "binary: ";
        for (int i = 0; i < binary.size(); ++i) {
            cout << binary[i] << ' ';
        }
        cout << "\nvalue: " << this->value << '\n';
        cout << "weight: " << this->weight << '\n';
        cout << "fitness: " << this->fitness << "\n";
    }

};

Chromosome generate_chromosome(vector<int> &value, vector<int> &weight) {
    vector<bool> binary(value.size());
    for (int i = 0; i < value.size(); ++i) {
        binary[i] = bool(rand() & 1);
    }
    return Chromosome(binary, value, weight);
}

// ------------------------------------------
/*---------------- */
vector<Chromosome> initial_population(vector<int> &value, vector<int> &weight, int n) {
    n = min(n, (1 << int(value.size())));
    vector<Chromosome> chromosomes;
    for (int c = 0; c < n; ++c) {
        Chromosome temp_chromosome = generate_chromosome(value, weight);
        if (temp_chromosome.get_fitness() == 0) {
            temp_chromosome.to_have_value(value, weight);
        }
        chromosomes.push_back(temp_chromosome);
    }
    return chromosomes;
}

// /*--------------------- Tournament selection -------------------*/

Chromosome dominant_chromosome(Chromosome &A, Chromosome &B) {
    return (A.get_fitness() >= B.get_fitness()) ? A : B;
}

Chromosome tournament_selection(vector<Chromosome> &population) { // select one parent form the old population
    int i = rand() % population.size();
    int j = rand() % population.size();
    return dominant_chromosome(population[i], population[j]);
}

pair<Chromosome, Chromosome>
cross_over(Chromosome &parent1, Chromosome &parent2, vector<int> &value, vector<int> &weight) {
    // todo: try different crossing points
    int crossing_point = parent1.get_length() >> 1; // divide by 2
    vector<bool> binary1 = parent1.get_binary();
    vector<bool> binary2 = parent2.get_binary();

    for (int i = crossing_point; i < parent1.get_length(); ++i) {
        swap(binary1[i], binary2[i]);
    }
    Chromosome c1 = Chromosome(binary1, value, weight);
    Chromosome c2 = Chromosome(binary2, value, weight);
    return {c1, c2};
}

bool dominant(Chromosome &c1, Chromosome &c2) {
    return c1.get_fitness() >= c2.get_fitness();
}
void sort_chromosome(vector<Chromosome> &v) {
    sort(v.begin(), v.end());
    reverse(v.begin(), v.end());
}


vector<Chromosome> init_new_pop(vector<Chromosome> old_population) { // Elitism
    sort_chromosome(old_population);
    vector<Chromosome> new_pop;
    for (int i = 0; i <= old_population.size() * 0.5; ++i) { // todo: changeable
        new_pop.push_back(old_population[i]);
    }
    return new_pop;
}

// ------------------------------------------


vector<Chromosome> get_new_population(vector<Chromosome> old_population, vector<int> &value, vector<int> &weight) {
    vector<Chromosome> ans = init_new_pop(old_population); // Elitism
    sort_chromosome(old_population);
    while (ans.size() < old_population.size()) {
        Chromosome parent1 = tournament_selection(old_population);
        Chromosome parent2 = tournament_selection(old_population);
        pair<Chromosome, Chromosome> children = cross_over(parent1, parent2, value,
                                                           weight); // needed for calculating new values in constructor

        children.first.mutate(value, weight);
        children.second.mutate(value, weight);

        if (children.first.get_fitness() == 0)
            children.first.to_have_value(value, weight);
        if (children.second.get_fitness() == 0)
            children.second.to_have_value(value, weight);

        ans.push_back(children.first);
        ans.push_back(children.second);
    }
    if (ans.size() > old_population.size()) {
        ans.pop_back();
    }
    return ans;
}

/*-------------- termination condition -------------*/
bool is_converged(vector<Chromosome> &pop) {
    sort_chromosome(pop);
    double cnt_similar_max = 0;
    for (Chromosome &c: pop) {
        if (c.get_fitness() == pop[0].get_fitness())
            cnt_similar_max++;
    }
    double n = pop.size();
    return ((cnt_similar_max / n * 100) >= 90);
}

bool swap_pop(vector<Chromosome> &old_pop, vector<Chromosome> &new_pop) {
    old_pop = new_pop;
    return true;
}

int main() {
    /*----------- Main Approach -----------*/
    /*
    old population = create parent population by randomly creating N individuals
    new population = create empty child population
    // add most dominant chromosomes from the old population --Elitism
    while not enough individuals in 'new population'
        parent1 = select parent   --tournament selection
        parent2 = select parent   --tournament selection
        child1, child2 = crossover(parent1, parent2)
        mutate child1, child2
        evaluate child1, child2 for fitness
        insert child1, child2 into new population
    end while
    'old population' = 'new population'
end while

 */

    srand(time(nullptr));
    vector<int> value {100, 10, 40, 37, 56, 47, 70, 34, 58, 15, 86, 83, 56, 85, 68, 45, 65,
                       44, 50, 43, 67, 85, 120, 33, 56, 90, 46, 250, 221, 11, 54, 77, 51, 88, 9, 77, 79, 89, 85, 4, 52,
                       55, 100, 33, 61, 70, 50, 102, 40, 30};
    vector<int> weight {60, 50, 10, 51, 88, 9, 77, 79, 89, 85, 4, 52, 55, 100, 33, 61, 77, 69, 30, 100, 40, 59, 28, 43,
                        56, 65, 63, 10, 55, 80, 60, 50, 40, 70, 20, 100, 45, 60, 112, 139, 123, 10, 7, 157, 90, 11,
                        50, 29, 46, 70};
    knapsack_size = 260;

    /*--------------------- Generating new generations ----------------------*/
    vector<Chromosome> old_pop = initial_population(value, weight, 100); // first population
    vector<Chromosome> new_pop; // next generation
    int cnt_generations = 0;
    /* print current population */
    cout << "Generation " << cnt_generations << ":\n";
    for (Chromosome &c: old_pop)
        cout << c.get_fitness() << ' ';
    cout << '\n';
    cnt_generations++;
    do {
        /* new generation */
        new_pop = get_new_population(old_pop, value, weight);
        sort_chromosome(new_pop);
        swap_pop(old_pop, new_pop);
        /* print current population */
        cout << "Generation " << cnt_generations << ":\n";
        for (Chromosome &c: new_pop)
            cout << c.get_fitness() << ' ';
        cout << '\n';
    } while (!is_converged(new_pop) && cnt_generations++ < MAX_GENERATIONS);
}
