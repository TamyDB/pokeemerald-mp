# Pokémon Emerald Multiplayer — Linux experimental

Ferramentas para recompilar o port C/SDL2 de Pokémon Emerald com multiplayer
ENet. O objetivo inicial é conectar jogadores e mostrar suas posições no mesmo
mapa. Checkpoint atual: **v0.1.0-alpha.3**, protocolo 2. IP e porta são configuração local e privada.

**A rede funciona nos testes automatizados pela internet. Ainda falta validar
visualmente dois jogadores dentro do jogo.** A compilação e a inicialização
Linux foram verificadas; controles, áudio e persistência do save ainda precisam
de teste manual nesta build. Chat, batalhas, trocas e QoL ficam para etapas futuras.

O GitHub contém apenas fontes, ferramentas, testes e documentação. **Não publicar
o jogo compilado, a ROM ou os pacotes locais no GitHub**, inclusive em Releases
ou artefatos CI. Consulte [a política de distribuição](docs/distribution.md).

## Rodar agora nesta máquina

A build já existe. Abra um terminal e execute:

```bash
cd /home/tamy/Projetos/pokeemerald-mp
./scripts/play_multiplayer_linux.sh
```

Antes da primeira execução, copie o modelo se ainda não houver configuração:

```bash
if [ ! -f config/multiplayer.env ]; then
    cp config/multiplayer.env.example config/multiplayer.env
fi
```

Abra `config/multiplayer.env` no seu editor e preencha o IPv4 e a porta UDP do
servidor que você deseja usar. Os dois campos vêm vazios no modelo:

```dotenv
POKE_NET_SERVER=
POKE_NET_PORT=
```

Esse arquivo preenchido é local e ignorado pelo Git. Não publique seu conteúdo.
O launcher recusa configuração ausente/inválida, sem usar um servidor padrão.
Inicie ou carregue um jogo e entre no mapa jogável. A conexão só começa no overworld; não espere uma
mensagem de conexão na tela de título.

No terminal, uma conexão bem-sucedida mostra algo semelhante a:

```text
NET: connecting to IP_CONFIGURADO:PORTA_CONFIGURADA
NET: handshake complete, player=... protocol=2
```

O número do jogador varia a cada sessão. O handshake confirma a conexão, mas
não comprova sozinho que outro jogador está sendo desenhado.

Para jogar sem iniciar conexão de rede:

```bash
env -u POKE_NET_SERVER -u POKE_NET_PORT ./scripts/play_linux.sh
```

Para substituir IP e porta temporariamente (exemplo com servidor local):

```bash
POKE_NET_SERVER=127.0.0.1 POKE_NET_PORT=25001 ./scripts/play_multiplayer_linux.sh
```

## Controles

| Ação | Tecla |
| --- | --- |
| Mover | Setas |
| A / confirmar | Z |
| B / voltar | X |
| Start | Enter |
| Select | Barra invertida (`\`) |
| L / R | A / S |
| Acelerar | Espaço |
| Pausar | Ctrl+P |

A build normal usa `build/linux-port/pokeemerald.sav` e `pokeemerald.cfg` nessa
mesma pasta. **Não execute duas instâncias que escrevam nesse mesmo save.**

## Testar dois jogadores no mesmo computador

Use duas pastas independentes. O procedimento abaixo cria uma pasta nova a cada
execução e não copia nem sobrescreve o save da build normal.

1. Abra dois terminais.
2. Em **cada terminal**, execute o bloco completo:

```bash
cd /home/tamy/Projetos/pokeemerald-mp
source scripts/multiplayer_config.sh
LoadMultiplayerConfig "$PWD/config/multiplayer.env"
mkdir -p build/playtests
test_dir="$(mktemp -d "$PWD/build/playtests/player.XXXXXX")"
cp build/linux-port/pokeemerald build/linux-port/BG*.png build/linux-port/Border.png "$test_dir/"
printf 'Pasta deste jogador: %s\n' "$test_dir"
cd "$test_dir"
./pokeemerald 2>&1 | tee network.log
```

3. Crie um jogo em cada janela. Para facilitar o encontro, escolha o mesmo tipo
   de personagem e avance até ambos estarem no mesmo lugar, por exemplo a rua
   de Littleroot. Interiores, andares e casas diferentes são mapas diferentes.
4. Confirme `NET: handshake complete` nos dois terminais, com IDs diferentes.
5. Deixe ambos no mapa, sem abrir menus, e alterne o foco entre as janelas.
6. Mova um jogador e observe a outra janela. Procure um sprite Brendan adicional
   mudando de tile e orientação. Nesta etapa não há nomes nem movimento suave.

Verifique e registre:

- O outro jogador aparece apenas no mesmo mapa?
- A posição e a orientação acompanham o movimento?
- Ao trocar de mapa, o sprite some do mapa anterior e reaparece quando ambos
  voltam ao mesmo lugar?
- Ao fechar uma janela, o outro sprite desaparece após a saída ser processada?
- NPCs locais continuam presentes e os comandos funcionam?
- É possível salvar pelo menu do jogo, fechar e reabrir o mesmo jogador?

Para reabrir um save de teste, use o caminho de pasta impresso no terminal:

```bash
cd /caminho/impresso/para/player.XXXXXX
POKE_NET_SERVER=SEU_IP POKE_NET_PORT=SUA_PORTA ./pokeemerald
```

Os logs ficam em `network.log` nas respectivas pastas. Se o teste falhar, registre
os dois logs, o lugar do jogo e o comportamento observado. Esses testes visuais
estão pendentes; não estão sendo declarados aprovados por este README.

## Testes automatizados

Na raiz do projeto:

```bash
make -f Makefile_pc network server relay-test test-network
make -f Makefile_pc test-network-loopback
python3 scripts/test_relay.py
./scripts/test_linux_smoke.sh
```

Para repetir a integração com o servidor público:

```bash
source scripts/multiplayer_config.sh
LoadMultiplayerConfig "$PWD/config/multiplayer.env"
build/pc-net/relay-test "$POKE_NET_SERVER" "$POKE_NET_PORT"
```

O resultado esperado é `Relay integration passed`. Esse teste cria até três
conexões temporárias; feche outras sessões se o servidor estiver cheio.

| Teste | O que verifica |
| --- | --- |
| `test-network` | Cliente, validação de pacotes, sequência, mapas e gerenciamento de slots |
| `test-network-loopback` | Transporte UDP real nos canais confiável e não confiável |
| `test_relay.py` | Servidor local com cliente real e dois probes ENet |
| `relay-test IP PORTA` | A mesma integração contra o servidor indicado |
| `test_linux_smoke.sh` | Inicialização até a cena inicial/quadro 300 e encerramento sem janela |

A integração usa callbacks de sprites simulados. O smoke test desliga imagem e
áudio. Eles complementam o teste manual; não o substituem.

## Recompilar o Linux

Nesta máquina, o port e as dependências já foram preparados:

```bash
./scripts/build_linux.sh
./scripts/play_multiplayer_linux.sh
```

A raiz contém a decompilação GBA. O `Makefile_pc` da raiz compila a camada de
rede, o servidor e os testes. O script acima compila o **jogo completo** no port
isolado em `build/linux-port`; não confundir os dois builds.

Para preparar novamente em outra máquina Fedora, o comando usado para instalar
as dependências foi:

```bash
sudo dnf install -y glibc-devel.i686 libgcc.i686 sdl2-compat-devel.i686 SDL2_image-devel.i686 mesa-libGL-devel.i686 mesa-libGLU-devel.i686 libpng-devel gcc-c++
```

Também são necessários Git, Python 3, Make e pkg-config. O port é **32 bits**,
mesmo em Linux x86_64; mantenha as bibliotecas i686.

Com `build/linux-port` ainda inexistente e uma ROM local compatível:

```bash
git clone https://github.com/gradenGnostic/pokeemerald-multiplatform.git /tmp/pokeemerald-linux-port
git -C /tmp/pokeemerald-linux-port checkout db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46
python3 scripts/prepare_linux.py --source /tmp/pokeemerald-linux-port --rom 'Pokemon - Emerald Version (USA, Europe).gba'
./scripts/build_linux.sh
```

O preparador exige um checkout upstream limpo nessa revisão e não sobrescreve uma
pasta de port existente. Se precisar refazer a preparação, preserve primeiro os
saves/configurações e mova a pasta anterior; não apague seu progresso.

A ROM é usada para conferir a revisão: SHA-1
`f3ae088181bf583e55daf962a92bb46f4f1d07b7`. Ela não é modificada nem publicada.
Instruções detalhadas e pacotes exclusivamente locais:
[docs/linux-playable.md](docs/linux-playable.md).

## Limitações e diagnóstico

- Polling ocorre no overworld; menus/batalhas podem suspender a comunicação.
- Não há reconexão automática. Após `NET: disconnected` ou timeout, feche e
  reabra o jogo para tentar novamente, depois volte ao overworld.
- Há até oito conexões totais no relay e sprites dependem de slots livres no mapa.
- Não há chat, batalhas/trocas multiplayer, nomes ou interpolação de movimento.
- Se conectou mas não vê o outro jogador, confira mapa, logs e se ambos continuam
  conectados. Essa parte ainda precisa da validação visual inicial.
- Se o executável não abrir, confira as bibliotecas com
  `ldd build/linux-port/pokeemerald`. As dependências precisam ser de 32 bits.
- Servidor local para desenvolvimento: compile com `make -f Makefile_pc server`
  e escolha uma porta, por exemplo `build/pc-net/pokeemerald-server 25001`; depois use
  `POKE_NET_SERVER=127.0.0.1 POKE_NET_PORT=25001 ./scripts/play_multiplayer_linux.sh`.

A VM de testes já está configurada e a conexão pública foi validada. IP e porta
não são publicados; obtenha-os com quem hospeda o servidor. Operação,
logs e comandos de serviço: [docs/server.md](docs/server.md).

## Contexto, decisões e checkpoints

- [ROADMAP.md](ROADMAP.md): status e propostas futuras.
- [CONTEXT.md](CONTEXT.md): objetivo, arquitetura e restrições permanentes.
- [WORK_SUMMARY.md](docs/WORK_SUMMARY.md): passagem de contexto e evidências.
- [Registro de decisões](docs/multiplayer-decisions.md): justificativas e validações.
- Checkpoints de fontes: `v0.1.0-alpha.1`, `v0.1.0-alpha.2` e `v0.1.0-alpha.3`.

Este trabalho parte da decompilação Pokémon Emerald e do port multiplataforma
indicado acima. Para o alvo GBA original, veja [INSTALL.md](INSTALL.md).
