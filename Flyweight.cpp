#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <random>
#include <iomanip>
#include <cstring>
#include <sstream>

static constexpr size_t TAMANHO_TEXTURA = 2 * 1024 * 1024; // 2mb por textura

class TipoArvore {
public:
    virtual ~TipoArvore() = default;

    virtual void renderizar(double x, double y, double altura, int galhos) const = 0;

    virtual const std::string& getEspecie() const = 0;
    virtual double getLarguraTronco() const = 0;
    virtual double getSombra() const = 0;
};

class TipoArvoreConcreto: public TipoArvore {
private:
    std::string especie_;
    double larguraTronco_;
    double sombra_;
    char textura_[TAMANHO_TEXTURA];

public:
    TipoArvoreConcreto(const std::string& especie, double larguraTronco, double sombraMaxima): especie_(especie), larguraTronco_(larguraTronco), sombra_(sombraMaxima) {
        std::memset(textura_, 0xAB, TAMANHO_TEXTURA);
    }

    void renderizar(double x, double y, double altura, int galhos) const override {
        (void)textura_[0];
        (void)x;
        (void)y;
        (void)altura;
        (void)galhos;
    }

    const std::string& getEspecie() const override {
        return especie_;
    }

    double getLarguraTronco() const override {
        return larguraTronco_;
    }

    double getSombra() const override {
        return sombraMaxima_;
    }
};

class FabricaTiposArvore {
private:
    std::unordered_map<std::string, std::shared_ptr<TipoArvore>> tipos_;

public:
    std::shared_ptr<TipoArvore> obterTipo(const std::string& especie, double larguraTronco, double sombraMaxima) {
        auto it = tipos_.find(especie);

        if (it != tipos_.end()) {
            return it->second;
        }

        auto novoTipo = std::make_shared<TipoArvoreConcreto>(
            especie,
            larguraTronco,
            sombraMaxima
        );

        tipos_[especie] = novoTipo;
        return novoTipo;
    }

    size_t quantidadeTipos() const {
        return tipos_.size();
    }

    size_t memoriaCompartilhada() const {
        return tipos_.size() * sizeof(TipoArvoreConcreto);
    }
};

struct Arvore {
    std::shared_ptr<TipoArvore> tipo; // compartilhado

    double x;
    double y;
    double altura;
    int galhos;

    void renderizar() const {
        tipo->renderizar(x, y, altura, galhos);
    }
};

struct ArvoreSemFlyweight {
    std::string especie;
    double larguraTronco;
    double sombraMaxima;
    char textura[TAMANHO_TEXTURA];

    double x;
    double y;
    double altura;
    int galhos;
};

static std::string formatarMemoria(size_t bytes) {
    std::ostringstream saida;
    saida << std::fixed << std::setprecision(2);

    if (bytes >= 1024ULL * 1024 * 1024) {
        saida << static_cast<double>(bytes) / (1024.0 * 1024 * 1024) << " GB";
    } else if (bytes >= 1024 * 1024) {
        saida << static_cast<double>(bytes) / (1024.0 * 1024) << " MB";
    } else {
        saida << bytes << " B";
    }

    return saida.str();
}

int main() {
    constexpr int QUANTIDADE_ESPECIES = 150;
    constexpr int QUANTIDADE_ARVORES = 10000;

    std::mt19937 gerador(42);

    struct DadosEspecie {
        std::string nome;
        double larguraTronco;
        double sombraMaxima;
    };

    std::vector<DadosEspecie> especies(QUANTIDADE_ESPECIES);

    for (int i = 0; i < QUANTIDADE_ESPECIES; i++) {
        especies[i] = {
            "Especie_" + std::to_string(i),
            std::uniform_real_distribution<>(0.1, 2.5)(gerador),
            std::uniform_real_distribution<>(2.0, 80.0)(gerador)
        };
    }

    FabricaTiposArvore fabrica;
    std::vector<Arvore> floresta;
    floresta.reserve(QUANTIDADE_ARVORES);

    std::uniform_int_distribution<> escolherEspecie(0, QUANTIDADE_ESPECIES - 1);
    std::uniform_real_distribution<> coordenada(-500.0, 500.0);
    std::uniform_real_distribution<> altura(1.0, 40.0);
    std::uniform_int_distribution<> quantidadeGalhos(3, 150);

    for (int i = 0; i < QUANTIDADE_ARVORES; i++) {
        const auto& especie = especies[escolherEspecie(gerador)];

        floresta.push_back({
            fabrica.obterTipo(
                especie.nome,
                especie.larguraTronco,
                especie.sombraMaxima
            ),
            coordenada(gerador),
            coordenada(gerador),
            altura(gerador),
            quantidadeGalhos(gerador)
        });
    }

    size_t memoriaPoolFlyweight = fabrica.memoriaCompartilhada();
    size_t memoriaArvoresFlyweight = QUANTIDADE_ARVORES * sizeof(Arvore);
    size_t memoriaTotalFlyweight = memoriaPoolFlyweight + memoriaArvoresFlyweight;

    size_t memoriaPorArvoreSemFlyweight = sizeof(ArvoreSemFlyweight);
    size_t memoriaTotalSemFlyweight =
        static_cast<size_t>(QUANTIDADE_ARVORES) * memoriaPorArvoreSemFlyweight;

    double percentualEconomia =
        (1.0 - static_cast<double>(memoriaTotalFlyweight) /
                   static_cast<double>(memoriaTotalSemFlyweight)) * 100.0;

    std::cout << "Quantidade de arvores e especies: \n";
    std::cout << QUANTIDADE_ESPECIES << " especies | "
              << QUANTIDADE_ARVORES << " arvores | textura = "
              << formatarMemoria(TAMANHO_TEXTURA) << " por especie\n\n";

    std::cout << "Sem Flyweight:\n";
    std::cout << "  Memoria por arvore: " << formatarMemoria(memoriaPorArvoreSemFlyweight) << "\n";
    std::cout << "  Memoria total:      " << formatarMemoria(memoriaTotalSemFlyweight) << "\n\n";

    std::cout << "Com Flyweight:\n";
    std::cout << "Compartilhado com "
              << fabrica.quantidadeTipos()
              << " tipos: " << formatarMemoria(memoriaPoolFlyweight) << "\n";

    std::cout << "  Memoria por arvore: " << sizeof(Arvore) << " B\n";
    std::cout << "  Memoria total:      " << formatarMemoria(memoriaTotalFlyweight) << "\n\n";

    std::cout << "Melhoria: "
              << formatarMemoria(memoriaTotalSemFlyweight - memoriaTotalFlyweight)
              << " (" << std::fixed << std::setprecision(1)
              << percentualEconomia << "%) — "
              << static_cast<double>(memoriaTotalSemFlyweight) /
                 static_cast<double>(memoriaTotalFlyweight)
              << "x menos memoria\n";

    return 0;
}