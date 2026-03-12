#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <random>
#include <iomanip>
#include <cmath>
#include <limits>

struct Item {
    std::string nome;
    int peso;
    int valor;
    double densidade;
    int id;
};

// struct para os resultados das heurísticas
struct GreedyResult {
    int valor;
    int peso;
    double ocupacao;
};

class KnapsackProblem {
private:
    std::vector<Item> itens;
    int capacidade;
    std::mt19937 rng; // Gerador de números aleatórios
    std::string str_peso;
    std::string str_valor;

public:
    std::map<std::string, GreedyResult> resultados_gulosos;

    KnapsackProblem(const std::map<std::string, std::map<std::string, int>>& dados_itens, 
                    int cap, 
                    int seed = 42, 
                    std::string s_peso = "peso", 
                    std::string s_valor = "valor") 
        : capacidade(cap), str_peso(s_peso), str_valor(s_valor) {
        
        rng.seed(seed);
        int idx = 0;
        for (auto const& [nome, specs] : dados_itens) {
            int p = specs.at(str_peso);
            int v = specs.at(str_valor);
            double d = (p > 0) ? static_cast<double>(v) / p : 0.0;
            itens.push_back({nome, p, v, d, idx++});
        }
    }

    std::pair<int, int> solucao_gulosa(std::string tipo_solucao, bool isDensidade = false) {
        std::vector<Item> itens_ordenados = itens;
        std::string nome_metodo;

        // Ordenação usando Lambdas
        if (tipo_solucao == str_peso || tipo_solucao == "peso") {
            std::sort(itens_ordenados.begin(), itens_ordenados.end(), [](const Item& a, const Item& b) {
                return (a.peso != b.peso) ? a.peso < b.peso : a.valor < b.valor;
            });
            nome_metodo = "Guloso (Tempo/Peso)";
        } 
        else if (tipo_solucao == str_valor || tipo_solucao == "valor") {
            std::sort(itens_ordenados.begin(), itens_ordenados.end(), [](const Item& a, const Item& b) {
                return (a.valor != b.valor) ? a.valor > b.valor : a.peso < b.peso;
            });
            nome_metodo = "Guloso (Lucro)";
        } 
        else if (tipo_solucao == "densidade") {
            std::sort(itens_ordenados.begin(), itens_ordenados.end(), [](const Item& a, const Item& b) {
                return (std::abs(a.densidade - b.densidade) > 1e-9) ? a.densidade > b.densidade : a.valor > b.valor;
            });
            nome_metodo = "Guloso (Densidade)";
        }

        int total_peso = 0;
        int soma_valores = 0;

        for (const auto& item : itens_ordenados) {
            if (total_peso + item.peso <= capacidade) {
                total_peso += item.peso;
                soma_valores += item.valor;

                std::cout << "Adicionando " << item.nome << " (" << str_peso << ": " << item.peso 
                          << " | " << str_valor << ": " << item.valor;
                if (isDensidade) std::cout << " | densidade: " << std::fixed << std::setprecision(2) << item.densidade;
                std::cout << ") | Total atual: " << total_peso << "\n";
            }
        }

        double ocupacao = (static_cast<double>(total_peso) / capacidade) * 100.0;
        resultados_gulosos[nome_metodo] = {soma_valores, total_peso, ocupacao};

        std::cout << "[" << nome_metodo << "] Valor Final: " << soma_valores << " | Peso Usado: " << total_peso << "\n\n";
        return {soma_valores, total_peso};
    }

    std::vector<int> get_solucao(std::string tipo) {
        int n = itens.size();
        std::vector<int> solucao(n, 0);

        if (tipo == "aleatoria") {
            std::uniform_int_distribution<int> dist(0, 1);
            for (int i = 0; i < n; ++i) solucao[i] = dist(rng);
        } 
        else if (tipo == "vazia") { /* já inicializado com 0 */ } 
        else if (tipo == "cheia") { std::fill(solucao.begin(), solucao.end(), 1); }
        else {
            // reusa a lógica de ordenação para soluções gulosas
            auto itens_ordenados = itens;
            if (tipo == "peso") {
                std::sort(itens_ordenados.begin(), itens_ordenados.end(), [](const Item& a, const Item& b) {
                    return a.peso < b.peso;
                });
            } else if (tipo == "valor") {
                std::sort(itens_ordenados.begin(), itens_ordenados.end(), [](const Item& a, const Item& b) {
                    return a.valor > b.valor;
                });
            } else if (tipo == "densidade") {
                std::sort(itens_ordenados.begin(), itens_ordenados.end(), [](const Item& a, const Item& b) {
                    return a.densidade > b.densidade;
                });
            }

            int peso_atual = 0;
            for (const auto& item : itens_ordenados) {
                if (peso_atual + item.peso <= capacidade) {
                    solucao[item.id] = 1;
                    peso_atual += item.peso;
                }
            }
        }
        return solucao;
    }

    // Retorna {valor_total, peso_total, avaliacao_penalizada}
    std::tuple<int, int, int> avaliar_solucao(const std::vector<int>& solucao, int taxa_violacao = 20) {
        int valor_total = 0;
        int peso_total = 0;

        for (size_t i = 0; i < solucao.size(); ++i) {
            if (solucao[i] == 1) {
                valor_total += itens[i].valor;
                peso_total += itens[i].peso;
            }
        }

        int avaliacao = valor_total;
        if (peso_total > capacidade) {
            int excesso = peso_total - capacidade;
            avaliacao -= (excesso * taxa_violacao);
        }

        return {valor_total, peso_total, avaliacao};
    }
};

int main() {
    std::map<std::string, std::map<std::string, int>> dados = {
        {"Item_A", {{"peso", 10}, {"valor", 60}}},
        {"Item_B", {{"peso", 20}, {"valor", 100}}},
        {"Item_C", {{"peso", 30}, {"valor", 120}}}
    };

    KnapsackProblem kp(dados, 50);
    kp.solucao_gulosa("densidade", true);

    auto sol = kp.get_solucao("aleatoria");
    auto [v, p, fit] = kp.avaliar_solucao(sol);

    std::cout << "Solução Aleatória - Valor: " << v << " Peso: " << p << " Fitness: " << fit << std::endl;

    return 0;
}