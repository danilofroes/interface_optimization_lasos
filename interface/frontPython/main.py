import sys
import os
import numpy as np

# Importando o módulo C++
dir_atual = os.path.dirname(os.path.abspath(__file__))
caminho_build = os.path.abspath(os.path.join(dir_atual, "..", "build", "Release"))
sys.path.append(caminho_build)

try:
    import meta_engine
except ImportError as e:
    print(f"X Erro ao importar o módulo C++: {e}")
    sys.exit(1)

# Importações da Interface Gráfica (PyQt6)
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                               QHBoxLayout, QFormLayout, QLineEdit, QPushButton, 
                               QTextEdit, QLabel)
from PyQt6.QtCore import Qt

import matplotlib
matplotlib.use('QtAgg') # Forçando o Matplotlib a usar o Qt correto
from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg as FigureCanvas
import matplotlib.pyplot as plt

# Função para gerar instâncias de produção aleatórias
def get_dict_producao(qntd_itens: int, tempo_max: int, lucro_max: int) -> dict:
    np.random.seed(42)
    dict_producao = {}
    for i in range(1, qntd_itens + 1):
        dict_producao[f'OP_{i:03d}'] = {
            'peso': int(np.random.randint(5, tempo_max)),
            'valor': int(np.random.randint(1000, lucro_max))
        }
    return dict_producao

# Interface Gráfica Principal
class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Otimizador com Metaheurísticas - LASOS")
        self.resize(1000, 600)

        widget_central = QWidget()
        self.setCentralWidget(widget_central)
        layout_principal = QHBoxLayout(widget_central)

        # Painel esquerdo para parâmetros e log
        painel_esquerdo = QWidget()
        layout_esq = QVBoxLayout(painel_esquerdo)
        layout_esq.setAlignment(Qt.AlignmentFlag.AlignTop)

        layout_esq.addWidget(QLabel("<b>Parâmetros do ILS</b>"))

        form_layout = QFormLayout()
        self.in_qntd = QLineEdit("500")
        self.in_capacidade = QLineEdit("480")
        self.in_iteracoes = QLineEdit("1000")
        self.in_perturbacao = QLineEdit("5")
        self.in_taxa_violacao = QLineEdit("20")
        self.in_limite_sem_melhora = QLineEdit("100")
        self.in_seed = QLineEdit("42")
        
        form_layout.addRow("Qtd. Itens:", self.in_qntd)
        form_layout.addRow("Capacidade (Turno):", self.in_capacidade)
        form_layout.addRow("Iterações:", self.in_iteracoes)
        form_layout.addRow("Perturbação:", self.in_perturbacao)
        form_layout.addRow("Taxa de Violação:", self.in_taxa_violacao)
        form_layout.addRow("Limite sem Melhora:", self.in_limite_sem_melhora)
        form_layout.addRow("Seed:", self.in_seed)
        layout_esq.addLayout(form_layout)

        self.btn_rodar = QPushButton("▶ Executar Otimização")
        self.btn_rodar.setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; padding: 10px;")
        self.btn_rodar.clicked.connect(self.rodar_otimizacao)
        layout_esq.addWidget(self.btn_rodar)

        self.caixa_log = QTextEdit()
        self.caixa_log.setReadOnly(True)
        layout_esq.addWidget(self.caixa_log)

        # Painel direito para visualização gráfica
        painel_direito = QWidget()
        layout_dir = QVBoxLayout(painel_direito)

        self.figura, self.ax = plt.subplots(figsize=(6, 4))
        self.canvas = FigureCanvas(self.figura)
        layout_dir.addWidget(self.canvas)

        layout_principal.addWidget(painel_esquerdo, 1)
        layout_principal.addWidget(painel_direito, 2)

    def log(self, mensagem):
        self.caixa_log.append(mensagem)
        QApplication.processEvents() 

    def rodar_otimizacao(self):
        self.caixa_log.clear()
        self.ax.clear()
        self.canvas.draw()

        qntd = int(self.in_qntd.text())
        capacidade = int(self.in_capacidade.text())
        iteracoes = int(self.in_iteracoes.text())
        perturbacao = int(self.in_perturbacao.text())
        taxa_violacao = int(self.in_taxa_violacao.text())
        limite_sem_melhora = int(self.in_limite_sem_melhora.text())
        seed = int(self.in_seed.text())

        self.log(f"Gerando {qntd} ordens de produção aleatórias com tempo máximo de 90 min e lucro máximo de R$ 5000...")
        ordens_producao = get_dict_producao(qntd, tempo_max=90, lucro_max=5000)

        config_ils = {
            "itens": ordens_producao,
            "capacidade": capacidade,
            "interacoes": iteracoes,
            "nivel_perturbacao": perturbacao,
            "taxa_violacao": taxa_violacao,
            "limite_sem_melhora": limite_sem_melhora,
            "seed": seed
        }

        self.log(f"Iniciando ILS... (backend em C++)")
        ils = meta_engine.ILS()
        ils.setParametros(config_ils)
        ils.solve()

        resultados = ils.getResultados()
        
        self.log("ILS finalizado!")
        self.log(f"Melhor Lucro: R$ {resultados.get('melhor_valor')}")
        self.log(f"Tempo Utilizado: {resultados.get('tempo_final_ils')}/{capacidade} min")
        
        historico = resultados.get("historico_ils", [])
        self.ax.plot(historico, color='#2980b9', linewidth=2)
        self.ax.set_title('Curva de Convergência (ILS C++)', fontsize=12)
        self.ax.set_xlabel('Iterações')
        self.ax.set_ylabel('Lucro (R$)')
        self.ax.grid(True, linestyle='--', alpha=0.7)
        self.canvas.draw()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    janela = MainWindow()
    janela.show()
    sys.exit(app.exec())