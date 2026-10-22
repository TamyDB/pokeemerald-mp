# Resumo do trabalho e passagem de contexto

Estado registrado em 2026-10-21, fuso America/Bahia. Leia primeiro
[CONTEXT.md](../CONTEXT.md) para objetivo, arquitetura e restrições permanentes.
Este documento registra uma fotografia do estado; confirme processos/serviços
novamente quando retomar uma sessão muito tempo depois.

## Resultado atual

Há um executável Linux i386 compilado, um relay ENet instalado na Oracle e testes
públicos aprovados. **Ainda não foi validada visualmente a sessão de dois jogadores
no jogo.** Gameplay prolongada, input, áudio e persistência do save permanecem
pendentes de teste manual. Não afirmar que o marco inteiro foi concluído.

## O que foi feito

1. Identificado que o checkout raiz é GBA e não contém o port SDL2 esperado.
2. ENet incorporado com revisão fixa e licença; Makefile GBA exclui `src/net`.
3. Criados protocolo, cliente, sincronização de Object Events e hook no overworld.
4. Adicionados testes de cliente, sincronização e transporte UDP real.
5. Obtido e preparado o port upstream em `build/linux-port`, sem substituir a raiz.
6. Conferida a ROM fornecida pelo usuário pelo hash esperado, sem modificá-la.
7. Usuário instalou as dependências i686 no Fedora; build Linux compilou.
8. Corrigidos conflitos de include/abs e integração SDL2_image com pkg-config.
9. Implementado relay C para oito conexões e protocolo 2 com JOIN confiável.
10. Instalados gcc/make/libc6-dev na VM, compilado e ativado o relay como serviço.
11. Adicionada regra local UDP ${POKE_NET_PORT} por serviço separado, preservando as demais.
12. Usuário adicionou a ingress rule na OCI; conectividade pública passou.
13. Corrigido travamento de encerramento descoberto pelo teste sem janela.
14. Criados roadmap, política de distribuição, scripts e pacotes estritamente locais.
15. Salvos commits e tags de checkpoint; nenhum push ou upload do jogo foi feito.

## Evidências e limites

| Verificação | Resultado observado | Limite |
| --- | --- | --- |
| Cliente determinístico | Handshake/canais, serialização, intervalo, validação, saída, timeout e erro de polling aprovados | ENet simulado |
| Sincronização | Sequência, duplicatas, rollover, troca de mapa, slots cheios e preservação de NPC aprovados | APIs de sprites simuladas |
| ENet loopback | Canal confiável e não confiável transmitiram dados reais | Não envolve gameplay |
| Relay local | Cliente real + dois probes: posições, entrada tardia, saída e reconexão aprovados | Callbacks de sprites simulados |
| Relay i386 | Mesmo teste com cliente de 32 bits e objetos ENet da build aprovado | Sem renderização |
| Relay na VM | Teste local da VM aprovado, inclusive após restart do serviço | Boot completo da VM não testado |
| Internet x86_64/i386 | Ambos passaram contra ${POKE_NET_SERVER}:${POKE_NET_PORT} após regra OCI | Não equivale a ver dois jogadores na tela |
| Firewall VM | Pacotes UDP ${POKE_NET_PORT} contados após abertura OCI | Não é teste de carga |
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

Workspace: `/home/tamy/Projetos/pokeemerald-mp`, Fedora 44.

- ROM local: `Pokemon - Emerald Version (USA, Europe).gba` — ignorada pelo Git.
- Port preparado e binário: `build/linux-port/pokeemerald`.
- Save normal: `build/linux-port/pokeemerald.sav`.
- Teste isolado: `build/linux-smoke`, com save/config próprios e `smoke.log`.
- Pacotes locais: `build/releases/pokeemerald-linux-v0.1.0-alpha.1.tar.gz` e alpha.2.
- Cada pacote inclui manifesto de hashes e commit de origem; nenhum foi publicado.

Duas instâncias locais precisam de diretórios separados para não escreverem no
mesmo save. Pode-se usar duas cópias locais do pacote; não publicar essas cópias.

## VM e operação

Host SSH configurado: `tet`; comando é **`ssh tet`**, não `tet` no shell.
Usuário ubuntu, IP público `${POKE_NET_SERVER}`, Ubuntu 24.04 x86_64, duas CPUs,
aproximadamente 1 GiB RAM. Recursos parecem suficientes para testes pequenos;
não há benchmark de oito jogadores.

- Fontes e binário: `/home/ubuntu/pokeemerald-server`.
- Relay: `build/pc-net/pokeemerald-server`, porta UDP ${POKE_NET_PORT}.
- Unit do usuário: `pokeemerald-server.service`, linger habilitado.
- Unit de sistema para firewall: `pokeemerald-udp.service`.
- Limites do relay: 128 MiB RAM e uma CPU; restart em falha.
- `SOURCE_CHECKPOINT` na VM registra alpha.1, origem dos fontes do servidor.
  Alpha.2 só atualizou documentação/validação pública, sem mudar o servidor.

```sh
ssh tet 'systemctl --user status pokeemerald-server'
ssh tet 'journalctl --user -u pokeemerald-server -n 50 --no-pager'
ssh tet 'systemctl --user restart pokeemerald-server'
```

A ingress rule OCI foi criada pelo usuário: origem 0.0.0.0/0, UDP, origem todas,
destino ${POKE_NET_PORT}, stateful. Já foi comprovada com tráfego público; não solicitar que
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
build/pc-net/relay-test ${POKE_NET_SERVER} ${POKE_NET_PORT}
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

## Checkpoints salvos

| Referência | Commit | Conteúdo |
| --- | --- | --- |
| v0.1.0-alpha.1 | 1e277ca47 | Base Linux/ENet, relay, testes, deploy e roadmap |
| v0.1.0-alpha.2 | 07b900050 | Conectividade pública comprovada; mesmo executável |
| Commit posterior | 82a5b662e | Política somente fontes/ferramentas no GitHub |

Branch: `multiplayer/initial-linux`. `VERSION` permanece `0.1.0-alpha.2`.
Novos commits de documentação vêm depois das tags; consultar `git log -5 --oneline`
para o HEAD atual. Para empacotar uma tag antiga, usar worktree/checkout limpo nessa
tag e reconstruir ali; o empacotador exige que a tag corresponda ao HEAD.

## Próximos passos recomendados

1. Testar duas instâncias/jogadores visualmente no mesmo mapa contra a VM.
2. Verificar movimento, orientação, troca de mapa, saída e preservação dos NPCs.
3. Verificar input, áudio e save sem compartilhar o mesmo diretório entre instâncias.
4. Registrar evidências e corrigir regressões antes de marcar o Marco 1 concluído.
5. Executar testes pertinentes, salvar commit e tag do próximo marco validado.
6. Só depois priorizar propostas de QoL/gameplay do roadmap com o usuário.

Limites conhecidos: sem reconexão automática, polling suspenso em menus/batalhas,
sprite Brendan por tile, sem interpolação, chat, batalhas ou trocas. Não ampliar
o escopo silenciosamente. Atualizar este resumo e o roadmap após mudanças relevantes.

## Guia de execução atualizado

O README principal agora ensina execução imediata, controles, recompilação,
testes automatizados e teste visual com dois jogadores. O bloco de teste manual
cria diretórios únicos em `build/playtests` e logs separados, preservando o save
da build normal. Os links locais e a sintaxe dos blocos Bash foram verificados.
Não houve nova validação visual; esse é o próximo teste a realizar com o usuário.
