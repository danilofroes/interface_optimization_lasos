#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <string>

struct Task {
    int machineID;
    int duration;
};

struct Job {
    std::string name;
    std::vector<Task> route;
    int dueDate;
};

const std::vector<Job> jobs = {
    {"P1", {{1, 3}, {2, 2}, {3, 2}}, 10}, // Carro: M1->M2->M3
    {"P2", {{2, 4}, {1, 1}, {3, 2}}, 8},  // Boneca: M2->M1->M3
    {"P3", {{3, 2}, {1, 4}, {2, 3}}, 12}  // Robô: M3->M1->M2
};

/// @brief Calcula o atraso total (Fitness) 
int calculateFitness(const std::vector<int>& chromosome) {
    std::vector<int> jobNextTask(jobs.size(), 0);
    std::vector<int> jobReadyTime(jobs.size(), 0);
    std::vector<int> machineFreeTime(4, 0); // M1, M2, M3
    std::vector<int> jobFinishTime(jobs.size(), 0);

    for (int jobId : chromosome) {
        int idx = jobId - 1;
        int taskIdx = jobNextTask[idx];
        Task t = jobs[idx].route[taskIdx];

        // respeita rota e disponibilidade da máquina 
        int startTime = std::max(machineFreeTime[t.machineID], jobReadyTime[idx]);
        int endTime = startTime + t.duration;

        machineFreeTime[t.machineID] = endTime;
        jobReadyTime[idx] = endTime;
        jobFinishTime[idx] = endTime;
        jobNextTask[idx]++;
    }

    int totalTardiness = 0;
    for (int i = 0; i < (int)jobs.size(); ++i) {
        totalTardiness += std::max(0, jobFinishTime[i] - jobs[i].dueDate);
    }

    return totalTardiness;
}

std::vector<int> orderCrossover(const std::vector<int>& p1, const std::vector<int>& p2, std::mt19937& g) {
    int size = p1.size();
    std::vector<int> child(size, -1);
    
    std::uniform_int_distribution<> dist(0, size - 1);
    int start = dist(g);
    int end = dist(g);
    if (start > end) std::swap(start, end);

    // Preserva trecho central
    for (int i = start; i <= end; ++i) child[i] = p1[i];

    // Preenche o restante com a ordem do Pai 2
    int current = (end + 1) % size;
    for (int i = 0; i < size; ++i) {
        int p2Idx = (end + 1 + i) % size;
        int item = p2[p2Idx];
        
        // Conta quantas vezes o item já aparece no filho para manter a validade
        if (std::count(child.begin(), child.end(), item) < 3) {
            child[current] = item;
            current = (current + 1) % size;
        }
    }
    return child;
}

// Mutação: Troca simples entre duas posições aleatórias
void mutate(std::vector<int>& chromosome, std::mt19937& g) {
    std::uniform_int_distribution<> dist(0, chromosome.size() - 1);
    std::swap(chromosome[dist(g)], chromosome[dist(g)]);
}

int main() {
    std::random_device rd;
    std::mt19937 g(rd());

    const int POP_SIZE = 20;
    const int GENERATIONS = 100;
    const float MUTATION_RATE = 0.1f;

    // Inicialização do cromossomo base: [1,1,1,2,2,2,3,3,3] 
    std::vector<int> base;
    for(int i=1; i<=3; ++i) for(int j=0; j<3; ++j) base.push_back(i);

    // Cria população inicial
    std::vector<std::vector<int>> population(POP_SIZE, base);
    for(auto& chromo : population) std::shuffle(chromo.begin(), chromo.end(), g);

    for (int gen = 0; gen < GENERATIONS; ++gen) {
        // Ordena por Fitness (Menor atraso é melhor)
        std::sort(population.begin(), population.end(), [](const auto& a, const auto& b) {
            return calculateFitness(a) < calculateFitness(b);
        });

        if (gen % 20 == 0) {
            std::cout << "Geracao " << gen << " | Melhor Atraso: " << calculateFitness(population[0]) << std::endl;
        }

        // Cria nova geração
        std::vector<std::vector<int>> nextGen;
        nextGen.push_back(population[0]); // Elitismo: mantém o melhor

        while (nextGen.size() < POP_SIZE) {
            // Seleção simples (Torneio entre os melhores da metade superior)
            std::uniform_int_distribution<> d(0, POP_SIZE / 2);
            auto& p1 = population[d(g)];
            auto& p2 = population[d(g)];

            auto child = orderCrossover(p1, p2, g);

            std::uniform_real_distribution<float> prob(0.0, 1.0);
            if (prob(g) < MUTATION_RATE) mutate(child, g);

            nextGen.push_back(child);
        }
        population = nextGen;
    }

    std::cout << "\n--- Resultado Final ---" << std::endl;
    std::cout << "Melhor Sequencia: ";
    for(int x : population[0]) std::cout << x << " ";
    std::cout << "\nAtraso Total: " << calculateFitness(population[0]) << " min" << std::endl;

    return 0;
}
