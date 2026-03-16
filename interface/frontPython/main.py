import sys
import os
import matplotlib.pyplot as plt

# 1. Configuração de caminhos para encontrar o .pyd / .so gerado pelo CMake
dir_atual = os.path.dirname(os.path.abspath(__file__))
caminho_build = os.path.abspath(os.path.join(dir_atual, "..", "build", "Release"))
sys.path.append(caminho_build)

try:
    import meta_engine
except ImportError as e:
    print(f"❌ Erro ao importar o módulo: {e}")
    sys.exit(1)

def plotar_convergencia(historico):
    """Plota a curva de evolução do ILS usando os dados retornados do C++"""
    if not historico:
        print("Sem dados no histórico para plotar.")
        return

    plt.figure(figsize=(10, 5))
    plt.plot(historico, label='Melhor Solução Global', color='#2980b9', linewidth=2)
    plt.title('Curva de Convergência (ILS processado em C++)', fontsize=14)
    plt.xlabel('Iterações', fontsize=12)
    plt.ylabel('Valor da Função Objetivo (Lucro)', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend()
    plt.show()

def main():
    print("🚀 Iniciando a comunicação com o Backend C++...")

    # 2. Definindo os dados do Problema da Mochila
    itens_teste = {
        "Item_A": {"peso": 12, "valor": 4},
        "Item_B": {"peso": 2, "valor": 2},
        "Item_C": {"peso": 1, "valor": 2},
        "Item_D": {"peso": 1, "valor": 1},
        "Item_E": {"peso": 4, "valor": 10},
        "Item_F": {"peso": 5, "valor": 8},
        "Item_G": {"peso": 2, "valor": 5},
    }

    config_ils = {
        "itens": itens_teste,
        "capacidade": 15,
        "interacoes": 200,          # Pode colocar milhares aqui depois que o C++ dá conta
        "nivel_perturbacao": 2,
        "taxa_violacao": 20,
        "limite_sem_melhora": 50,
        "seed": 42
    }

    # 3. Instanciando a classe C++ no Python
    ils = meta_engine.ILS()
    print(f"✅ Algoritmo instanciado: {ils.getNome()}")
    
    # Passamos o dicionário; o pybind11 + nlohmann transformam isso em JSON nativo no C++
    ils.setParametros(config_ils)

    # 4. Executando a otimização
    print("⏳ Rodando a meta-heurística... ", end="", flush=True)
    ils.solve()
    print("Concluído!")

    # 5. Coletando os resultados
    resultados = ils.getResultados()
    
    melhor_valor = resultados.get("melhor_valor")
    peso_usado = resultados.get("tempo_final_ils")
    itens_selecionados = resultados.get("itens_selecionados", [])
    historico = resultados.get("historico_ils", [])

    print("\n🏆 Resultados da Otimização:")
    print(f"Lucro Máximo: R$ {melhor_valor}")
    print(f"Peso Utilizado: {peso_usado} / {config_ils['capacidade']}")
    print(f"Itens Selecionados: {', '.join(itens_selecionados)}")

    # 6. Plotando o gráfico com os dados puros gerados no backend
    print("\n📊 Gerando gráfico de convergência...")
    plotar_convergencia(historico)

if __name__ == "__main__":
    main()