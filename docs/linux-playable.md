# Build Linux

[Início](../README.md) · [Documentação](README.md) · [Servidor](server.md) · [Versionamento](versioning.md)

Guia técnico da preparação e compilação do port SDL2 integrado à camada multiplayer.
Para a instalação guiada, controles e teste com dois jogadores, comece pelo
[README](../README.md#como-executar). Execute os comandos a partir da raiz do projeto.

## Estado e requisitos

| Item | Referência |
| --- | --- |
| Ambiente usado na validação | Fedora 44 x86_64 com bibliotecas i686 |
| Executável | Linux ELF i386 de 32 bits, em `build/linux-port/pokeemerald` |
| Port upstream | `gradenGnostic/pokeemerald-multiplatform` |
| Revisão fixa | `db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46` |
| ROM suportada | Emerald USA/Europe, SHA-1 `f3ae088181bf583e55daf962a92bb46f4f1d07b7` |

A ROM apenas verifica a revisão: não é modificada nem copiada. O executável é
compilado dos fontes. A raiz permanece como decompilação GBA; o port é integrado
em uma pasta gerada, ignorada pelo Git.

**Limite da validação:** inicialização sem janela, até a cena inicial/quadro 300,
e encerramento foram testados. Imagem, áudio, controles, save e presença visual
de dois jogadores continuam pendentes. Evidências: [resumo de trabalho](WORK_SUMMARY.md).

## 1. Instalar dependências

No Fedora, com ferramentas e bibliotecas de 32 bits:

```bash
sudo dnf install -y git python3 make gcc gcc-c++ pkgconf-pkg-config file \
    glibc-devel.i686 libgcc.i686 sdl2-compat-devel.i686 \
    SDL2_image-devel.i686 mesa-libGL-devel.i686 mesa-libGLU-devel.i686 libpng-devel
```

O port depende de ponteiros de 32 bits: mantenha `-m32`. O script de build
seleciona metadados pkg-config i686 em `/usr/lib/pkgconfig` quando disponíveis.
Outras distribuições precisam dos equivalentes e não foram validadas nesta entrega.

## 2. Preparar o port

Com o destino `build/linux-port` ainda inexistente:

```bash
git clone https://github.com/gradenGnostic/pokeemerald-multiplatform.git /tmp/pokeemerald-linux-port
git -C /tmp/pokeemerald-linux-port checkout db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46
python3 scripts/prepare_linux.py \
    --source /tmp/pokeemerald-linux-port \
    --rom '/caminho/para/Pokemon - Emerald Version (USA, Europe).gba'
```

Substitua o caminho da ROM pelo seu arquivo. Se a pasta temporária já existir,
use outro destino e ajuste `--source`. O preparador recusa revisão inesperada,
checkout upstream modificado, ROM incompatível e destino existente. Os patches
verificam os pontos de integração antes de alterá-los.

Para refazer uma preparação, preserve saves e configurações e mova a pasta
anterior. Não apague seu progresso para resolver uma falha de build.

## 3. Compilar e executar

```bash
./scripts/build_linux.sh
env -u POKE_NET_SERVER -u POKE_NET_PORT ./scripts/play_linux.sh
```

O build sincroniza os fontes de rede versionados com o port e confere o ELF e as
bibliotecas com `file` e `ldd`. O launcher entra na pasta do port para encontrar
arte, configuração e save.

| Arquivo | Uso |
| --- | --- |
| `build/linux-port/pokeemerald` | Executável |
| `build/linux-port/pokeemerald.sav` | Save normal |
| `build/linux-port/pokeemerald.cfg` | Configuração do port |
| `config/multiplayer.env` | IPv4 e porta privados do launcher multiplayer |

Para rede, siga a [configuração do README](../README.md#4-configurar-e-iniciar)
e execute `./scripts/play_multiplayer_linux.sh`. A conexão começa no overworld,
com servidor compatível com o protocolo 2. Não há servidor padrão.
Duas instâncias devem usar diretórios diferentes para não compartilhar save/config.

## 4. Verificar a build

```bash
make -f Makefile_pc network server relay-test test-network
make -f Makefile_pc test-network-loopback
python3 scripts/test_relay.py
./scripts/test_linux_smoke.sh
```

O smoke test usa pasta isolada, sem janela nem áudio. O log fica em
`build/linux-smoke/smoke.log`; o timeout esperado é 124, após oito segundos.
Ele não verifica renderização, input, áudio ou persistência.

Checklist manual, ainda pendente:

- [ ] Abrir o jogo e chegar ao overworld.
- [ ] Verificar imagem, teclado e áudio.
- [ ] Salvar, fechar e reabrir para conferir persistência.
- [ ] Observar dois jogadores no mesmo mapa, com saves separados.
- [ ] Verificar movimento, orientação, mudança de mapa, saída e NPCs.

Use o [roteiro de teste manual](../README.md#verificar-o-funcionamento) e registre
logs e resultados antes de marcar os itens como concluídos.

## Checkpoints e pacotes locais

Consulte o [changelog](../CHANGELOG.md) para as diferenças entre alpha.1, alpha.2
e alpha.3, e o [guia de versionamento](versioning.md#empacotar-localmente) para
requisitos e comandos de empacotamento.

Pacotes em `build/releases/` são exclusivamente locais. Não publicar jogo
compilado, ROM ou save no GitHub. A configuração privada não acompanha o pacote;
o modelo tem campos vazios. No pacote local, `./play.sh --offline` inicia sem rede.
