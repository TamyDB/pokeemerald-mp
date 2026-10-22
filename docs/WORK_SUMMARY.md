# Resumo de trabalho e retomada

[Início](../README.md) · [Documentação](README.md) · [Contexto](../CONTEXT.md) · [Histórico de trabalho](history/2026-10-21.md)

**Organização documental:** 22/10/2026 · **Últimas evidências operacionais:** 21/10/2026, America/Bahia.

Esta página consolida o ponto de retomada. Serviços e processos não foram
reconsultados durante a organização documental. O relato completo da sessão
anterior está preservado no histórico; resultados antigos não comprovam o estado
atual da VM.

## Resultado atual

Há um executável Linux i386 compilado e um relay ENet instalado na Oracle.
Os testes públicos passaram com clientes x86_64 e i386. **A sessão visual com dois
jogadores ainda não foi validada.** Input, áudio, save e sessões prolongadas
permanecem pendentes de teste manual.

Checkpoint declarado: `v0.1.0-alpha.3`, protocolo 2. O launcher exige IP e porta
privados por configuração local. O branch `multiplayer/initial-linux` e as três
tags alpha foram publicados com autorização, após sanitização; nenhuma build,
ROM, configuração pessoal ou valor de endpoint foi publicado. O master foi preservado.

## Próximos passos

1. Seguir o teste de duas instâncias no [README](../README.md#verificar-o-funcionamento), com pastas e saves separados.
2. Confirmar presença, movimento, orientação, troca de mapa, saída e NPCs.
3. Verificar controles, áudio e save; registrar logs e observações.
4. Corrigir regressões, executar os testes pertinentes e atualizar o roadmap.
5. Salvar o próximo checkpoint somente com evidências e limites explícitos.

Menu Multiplayer, novas configurações e nomes próprios com inicial maiúscula
continuam no [backlog](../ROADMAP.md); não foram implementados nessa sessão.

## Evidências e limites

| Verificação | Resultado observado | Limite |
| --- | --- | --- |
| Cliente determinístico | Handshake/canais, serialização, intervalo, validação, saída, timeout e erro de polling aprovados | ENet simulado |
| Sincronização | Sequência, duplicatas, rollover, troca de mapa, slots cheios e preservação de NPC aprovados | APIs de sprites simuladas |
| ENet loopback | Canal confiável e não confiável transmitiram dados reais | Não envolve gameplay |
| Relay local | Cliente real + dois probes: posições, entrada tardia, saída e reconexão aprovados | Callbacks de sprites simulados |
| Relay i386 | Mesmo teste com cliente de 32 bits e objetos ENet da build aprovado | Sem renderização |
| Relay na VM | Teste local da VM aprovado, inclusive após restart do serviço | Boot completo da VM não testado |
| Internet x86_64/i386 | Ambos passaram contra $POKE_NET_SERVER:$POKE_NET_PORT após regra OCI | Não equivale a ver dois jogadores na tela |
| Firewall VM | Pacotes UDP $POKE_NET_PORT contados após abertura OCI | Não é teste de carga |
| Executável Linux | ELF i386; ldd resolveu bibliotecas; cena inicial e quadro 300 em teste de oito segundos | Sem inspeção visual/áudio/input |
| Encerramento | Nova build terminou após o timeout sem precisar de SIGKILL | Sessões prolongadas ainda não testadas |
| Pacotes locais | Manifestos SHA-256 conferidos; executável igual ao testado | Não incluem bibliotecas do sistema |

## Arquivos centrais

- `include/net/net_client.h`: protocolo 2, tipos e APIs.
- `src/net/net_client.c`: conexão, handshake, polling, membros e envio de posições.
- `src/net/net_sync.c`: posições remotas e Object Events.
- `src/overworld.c`, `src/event_object_movement.c`: hooks protegidos por MULTIPLAYER.
- `server/main.c`: relay independente, sem assets ou memória do jogo.
- `Makefile_pc`: biblioteca de rede, servidor e testes; não compila o port completo.
- `scripts/prepare_linux.py`: aplica integração ao port upstream fixo e limpo.
- `scripts/build_linux.sh`: sincroniza fontes de rede no port e compila o Linux.
- `scripts/play_linux.sh`: jogo local; `play_multiplayer_linux.sh`: configura a VM.
- `scripts/test_linux_smoke.sh`, `scripts/test_relay.py`: verificações operacionais.
- `scripts/package_linux.py`: pacote local, exige checkout limpo e tag no HEAD.
- `deploy/`: units do relay e da regra local UDP.

## Ambiente local e artefatos

Checkout: pasta escolhida ao clonar o projeto. Ambiente validado: Fedora 44.

- ROM local: `Pokemon - Emerald Version (USA, Europe).gba` — ignorada pelo Git.
- Port preparado e binário: `build/linux-port/pokeemerald`.
- Save normal: `build/linux-port/pokeemerald.sav`.
- Teste isolado: `build/linux-smoke`, com save/config próprios e `smoke.log`.
- Pacotes locais: `build/releases/pokeemerald-linux-v0.1.0-alpha.1.tar.gz` e alpha.2.
- Cada pacote inclui manifesto de hashes e commit de origem; nenhum foi publicado.

Duas instâncias locais precisam de diretórios separados para não escreverem no
mesmo save. Pode-se usar duas cópias locais do pacote; não publicar essas cópias.

## VM e operação

Os comandos abaixo usam `SSH_HOST`, que deve conter um host ou apelido SSH
configurado por quem administra o servidor. Não há apelido obrigatório.
Na instalação de referência: Ubuntu 24.04 x86_64, duas CPUs, endpoint privado na configuração local ignorada pelo Git,
aproximadamente 1 GiB RAM. Recursos parecem suficientes para testes pequenos;
não há benchmark de oito jogadores.

- Fontes e binário: `~/pokeemerald-server` no usuário remoto.
- Relay: `build/pc-net/pokeemerald-server`, porta UDP $POKE_NET_PORT.
- Unit do usuário: `pokeemerald-server.service`, linger habilitado.
- Unit de sistema para firewall: `pokeemerald-udp.service`.
- Limites do relay: 128 MiB RAM e uma CPU; restart em falha.
- `SOURCE_CHECKPOINT` na VM registra alpha.1, origem dos fontes do servidor.
  Alpha.2 só atualizou documentação/validação pública, sem mudar o servidor.

```bash
# Defina seu destino SSH antes de executar os comandos.
SSH_HOST='usuario@servidor'
ssh "$SSH_HOST" 'systemctl --user status pokeemerald-server'
ssh "$SSH_HOST" 'journalctl --user -u pokeemerald-server -n 50 --no-pager'
ssh "$SSH_HOST" 'systemctl --user restart pokeemerald-server'
```

A ingress rule OCI foi criada pelo usuário: origem 0.0.0.0/0, UDP, origem todas,
destino $POKE_NET_PORT, stateful. Já foi comprovada com tráfego público; não solicitar que
seja recriada sem evidência de regressão. O painel/API OCI não foi acessado.
Não registrar chaves privadas ou credenciais na documentação.

## Comandos de reprodução

Com o port já preparado:

```sh
make -f Makefile_pc network server relay-test test-network
make -f Makefile_pc test-network-loopback
python3 scripts/test_relay.py
./scripts/build_linux.sh
./scripts/test_linux_smoke.sh
./scripts/play_multiplayer_linux.sh
```

Para repetir o teste público (ocupa até três conexões temporárias):

```sh
build/pc-net/relay-test $POKE_NET_SERVER $POKE_NET_PORT
```

Para recriar o teste i386, após compilar o port:

```sh
cc -m32 -std=gnu99 -UNDEBUG -iquote include -Iinclude/net tests/net/relay_test.c src/net/net_client.c build/linux-port/build/linux/src/net/enet/*.o -lpthread -o build/pc-net/relay-test32
python3 scripts/test_relay.py --client build/pc-net/relay-test32
```

No sandbox desta sessão, sockets e execução/ldd de i386 exigiram execução fora
do sandbox; no terminal normal do usuário funcionam com as dependências instaladas.
Não interpretar esse bloqueio como defeito do jogo ou dependência ausente.
ENet foi mantido como upstream, inclusive whitespace; checks de formatação dos
fontes próprios podem excluir `src/net/enet` e `include/net/enet`.

## Configuração e referências de versão

`config/multiplayer.env` é privado e ignorado pelo Git. O modelo público vem com
campos vazios; o launcher valida IPv4 e porta, sem executar código do arquivo.
A configuração do ambiente tem prioridade. O serviço existente da VM manteve
seus valores privados; os templates públicos foram parametrizados.

Na alpha.3, testes de configuração e cliente, integração com porta efêmera,
recompilação Linux e smoke test offline foram aprovados. Isso não acrescenta
validação visual ao primeiro marco.

A sanitização anterior à publicação alterou hashes locais. Manifestos antigos e
o `SOURCE_CHECKPOINT` da VM podem referenciar hashes anteriores; consulte as tags
publicadas pelo nome e o Git para obter a referência atual. Não mover tags nem
refazer a sanitização sem necessidade.

Histórico por checkpoint: [changelog](../CHANGELOG.md). Procedimentos de tags e
pacotes: [versionamento](versioning.md).

## Limitações conhecidas

Sem reconexão automática; polling suspenso em menus/batalhas; sprite Brendan por
tile, sem interpolação, chat, batalhas ou trocas multiplayer. O teste com oito
jogadores e o boot completo da VM não foram validados.

## Organização documental de 22/10/2026

README reorganizado para apresentação, instalação, arquitetura e objetivos.
Documentação agrupada por finalidade em um índice; criados changelog e guia de
versionamento; a passagem de contexto anterior foi arquivada com os caminhos e destinos SSH generalizados.
Essa atualização é documental: não declara novos testes de gameplay ou rede,
não altera VERSION nem cria uma nova tag.
