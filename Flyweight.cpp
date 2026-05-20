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