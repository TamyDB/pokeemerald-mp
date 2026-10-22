# Versão jogável nativa de Linux

## Estado atual

A ROM local foi verificada: SHA-1
`f3ae088181bf583e55daf962a92bb46f4f1d07b7`, Emerald USA/Europe.
Ela foi usada para conferir a revisão do jogo; não é modificada nem copiada.
O executável nativo é compilado a partir do código-fonte, não convertido da ROM.

O port SDL2 foi obtido de `gradenGnostic/pokeemerald-multiplatform`, commit
`db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46`, e preparado em `build/linux-port`.
Essa pasta contém a camada ENet deste projeto e hooks no overworld/encerramento.
As fontes originais da raiz são preservadas.

O executável foi compilado com sucesso: `build/linux-port/pokeemerald`, ELF
Linux Intel i386 de 32 bits. A verificação com `ldd` resolveu todas as bibliotecas.
O teste sem janela, sem áudio e em pasta de save isolada rodou por oito segundos,
chegou à cena inicial e ao quadro 300. Foi encerrado pelo timeout esperado (124).
Imagem, áudio, controles, criação de jogo e salvamento ainda exigem teste manual.

## Preparação reproduzível

No Fedora, instale pelo seu terminal:

```sh
sudo dnf install -y glibc-devel.i686 libgcc.i686 sdl2-compat-devel.i686 SDL2_image-devel.i686 mesa-libGL-devel.i686 mesa-libGLU-devel.i686 libpng-devel gcc-c++
```

A preparação já foi executada nesta máquina. Para reproduzir em outro checkout,
com `build/linux-port` ainda inexistente:

```sh
git clone https://github.com/gradenGnostic/pokeemerald-multiplatform.git /tmp/pokeemerald-linux-port
git -C /tmp/pokeemerald-linux-port checkout db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46
python3 scripts/prepare_linux.py --source /tmp/pokeemerald-linux-port --rom 'Pokemon - Emerald Version (USA, Europe).gba'
```

O script recusa revisão inesperada, fonte modificada, ROM incompatível e pasta
de destino existente. Os patches verificam os pontos de integração upstream.

## Compilar e jogar

```sh
./scripts/build_linux.sh
./scripts/play_linux.sh
```

O launcher usa a pasta do port como diretório de trabalho para encontrar arte,
configuração e save. O save fica em `build/linux-port/pokeemerald.sav`.
Controles: setas, Z (A), X (B), Enter (Start), barra invertida (Select), A/S (L/R).

O jogo inicia em modo local. Para tentar o cliente de rede ao entrar no overworld:

```sh
POKE_NET_SERVER=127.0.0.1 ./scripts/play_linux.sh
```

A porta é ${POKE_NET_PORT}. Exige servidor compatível com o protocolo de `multiplayer.md`;
o servidor está ativo na VM e os testes públicos passaram em x86_64 e i386.
Sem essa variável não inicia conexão. Para usar a VM, execute
`./scripts/play_multiplayer_linux.sh`.

## Validação prevista

- Compilar e conferir ELF de 32 bits e bibliotecas com `file` e `ldd`.
- Iniciar, chegar ao título, criar jogo e entrar no overworld.
- Verificar teclado, imagem e áudio.
- Salvar, fechar e reabrir para conferir persistência.
- Testar rede com servidor e duas instâncias quando disponíveis.

Os testes manuais no jogo estão pendentes. Para repetir a inicialização sem janela,
execute `./scripts/test_linux_smoke.sh`; o log fica em
`build/linux-smoke/smoke.log`. Esse teste não verifica a apresentação visual ou o áudio.

Os testes automatizados da camada de rede
são executados separadamente com `make -f Makefile_pc network test-network`.

## Checkpoint e pacote local

Versão inicial: `v0.1.0-alpha.1`; checkpoint com rede pública validada:
`v0.1.0-alpha.2`, branch `multiplayer/initial-linux`. O executável é o mesmo;
a segunda versão registra a nova evidência de conectividade.
O marco multiplayer completo ainda depende dos itens abertos em ROADMAP.md.

Depois de fazer checkout da tag e compilar:

```sh
python3 scripts/package_linux.py v0.1.0-alpha.2
```

O pacote fica em `build/releases/`, com launcher, arte, licenças e manifesto
SHA-256 ligado ao commit e à revisão do port. Não inclui ROM, save ou bibliotecas
de sistema. O script exige fontes limpas e uma tag que identifique o commit atual.

Para consultar o checkpoint sem alterar seu trabalho em andamento:

```sh
git show v0.1.0-alpha.1
git worktree add ../pokeemerald-checkpoint v0.1.0-alpha.1
```

As tags e commits foram criados localmente. Não houve push para o remoto.
