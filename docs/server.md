# Servidor relay ENet

[Início](../README.md) · [Documentação](README.md) · [Protocolo](multiplayer.md) · [Operação registrada](WORK_SUMMARY.md#vm-e-operação)

O relay em `server/main.c` encaminha presença e posições de até **oito conexões
totais**. Não carrega ROM, save, scripts de mapa ou dados de Pokémon. É experimental:
não valida progressão ou colisão e não fornece autenticação de jogadores.

## Navegação

- [Compilar e executar](#compilar-e-executar)
- [Testar o relay](#testar-o-relay)
- [Contrato do protocolo](#contrato-do-protocolo)
- [Serviços e operação remota](#serviços-e-operação-remota)
- [Rede e firewall](#rede-e-firewall)
- [Conectar o jogo](#conectar-o-jogo)

## Compilar e executar

Na raiz do projeto, com compilador C, Make e Python 3 disponíveis:

```bash
make -f Makefile_pc server
build/pc-net/pokeemerald-server 25001
```

A porta é explícita; `25001` é um exemplo local. Mantenha o terminal aberto ou
configure um serviço. Para conectar no mesmo computador:

```bash
POKE_NET_SERVER=127.0.0.1 POKE_NET_PORT=25001 ./scripts/play_multiplayer_linux.sh
```

O relay não depende das bibliotecas gráficas i686 do jogo. A implantação de
referência foi compilada no Ubuntu 24.04 x86_64 com GCC, Make e libc6-dev.

## Testar o relay

```bash
make -f Makefile_pc server relay-test test-network
python3 scripts/test_relay.py
```

A integração usa o cliente real do jogo com callbacks de sincronização simulados
e dois probes ENet. Verifica handshake, IDs, posições, pacotes antigos, campos
inválidos, canal incorreto, entrada tardia, saída e reconexão.
Requer sockets UDP; não verifica sprites na tela.

Para testar outro servidor, preencha a configuração local conforme o
[README](../README.md#4-configurar-e-iniciar) e execute:

```bash
source scripts/multiplayer_config.sh
LoadMultiplayerConfig "$PWD/config/multiplayer.env"
build/pc-net/relay-test "$POKE_NET_SERVER" "$POKE_NET_PORT"
```

O resultado esperado é `Relay integration passed`. O teste ocupa até três
conexões temporárias; confira as vagas disponíveis antes de executá-lo.

## Contrato do protocolo

| Etapa | Comportamento |
| --- | --- |
| Conexão | Connect data ENet e HELLO informam protocolo 2 |
| HELLO / WELCOME | Canal 1 confiável; servidor atribui ID exclusivo de sua execução |
| JOIN | Canal 1 confiável estabelece membros cujas posições podem ser aceitas |
| POSITION | Canal 0; servidor valida campos e sequência e aceita até 40 posições/s por conexão |
| Snapshot | Repetido a cada 50 ms para recuperar estado de entradas tardias e posições anteriores ao JOIN |
| LEAVE | Canal 1 confiável remove o membro; novas posições daquele ID são ignoradas sem JOIN válido |

Inteiros multibyte são big endian; tipos e tamanhos estão em
`include/net/net_client.h`. IDs não são reutilizados na mesma execução e não são
escolhidos pelo remetente. Clientes sem handshake saem após dez segundos.
Clientes de protocolo 1 não são compatíveis. Detalhes: [protocolo](multiplayer.md).

O cliente tem até oito slots remotos, sujeitos a Object Events livres; o limite
de oito conexões do relay inclui o jogador local. Chat, batalhas e trocas não
estão implementados.

## Serviços e operação remota

Os templates versionados descrevem a instalação de referência; não são um
instalador automático. Ajuste usuário, caminhos e porta ao ambiente de destino.

| Arquivo | Responsabilidade |
| --- | --- |
| `deploy/pokeemerald-server.service` | Serviço systemd de usuário para o relay |
| `deploy/pokeemerald-udp.service` | Serviço de sistema para a regra UDP local |
| `config/server.env.example` | Modelo sem porta privada preenchida |
| `~/.config/pokeemerald-server/server.env` | Porta do serviço de usuário no servidor |
| `/etc/pokeemerald-server.env` | Porta da regra local, administrada por root |

Os dois arquivos privados precisam indicar a mesma porta em `POKE_NET_PORT`.
Não publicar seus valores. A referência usa fontes em `~/pokeemerald-server` no
usuário remoto e binário relativo `build/pc-net/pokeemerald-server`.
O serviço tem restart em falha, limite de 128 MiB e uma CPU. Linger foi habilitado
na instalação de referência para execução sem login interativo.

Para operar uma instalação já configurada, substitua o destino SSH abaixo:

```bash
SSH_HOST='usuario@servidor'
ssh "$SSH_HOST" 'systemctl --user status pokeemerald-server'
ssh "$SSH_HOST" 'journalctl --user -u pokeemerald-server -n 50 --no-pager'
ssh "$SSH_HOST" 'systemctl --user restart pokeemerald-server'
```

Também pode usar um apelido do seu `~/.ssh/config`. O destino SSH é configuração
pessoal; nenhum apelido específico é exigido pelo projeto.

No terminal remoto, quando necessário:

| Ação | Comando |
| --- | --- |
| Parar o relay | `systemctl --user stop pokeemerald-server` |
| Desabilitar o relay | `systemctl --user disable --now pokeemerald-server` |
| Desabilitar a liberação UDP local | `sudo systemctl disable --now pokeemerald-udp` |

Essas ações afetam sessões conectadas. O reinício do serviço foi validado em
21/10/2026; boot completo da VM e carga de oito jogadores não foram testados.

## Rede e firewall

Para conexões externas, a porta UDP escolhida precisa estar acessível no sistema
e na infraestrutura. Se estiver atrás de NAT, confira também o encaminhamento
da porta. Não é necessário abrir TCP para o relay ENet.

Na implantação de referência Oracle, a regra de entrada foi:

| Campo | Valor de referência |
| --- | --- |
| Origem CIDR | `0.0.0.0/0` |
| Protocolo | UDP |
| Porta de origem | Todas |
| Porta de destino | A configurada em `POKE_NET_PORT` |
| Tipo | Stateful |

Essa regra já foi aplicada pelo administrador e os testes públicos passaram em
x86_64 e i386. Não há motivo para recriá-la sem evidência de regressão.
O painel/API OCI não foi acessado durante a implementação. Referência herdada:
[Security Lists da Oracle](https://docs.oracle.com/en-us/iaas/Content/Network/Concepts/creating-securitylist.htm).

## Conectar o jogo

O jogador preenche `config/multiplayer.env` a partir do modelo público ou fornece
`POKE_NET_SERVER` e `POKE_NET_PORT` no ambiente. O launcher não tem servidor padrão:

```bash
./scripts/play_multiplayer_linux.sh
```

A conexão começa no overworld. Duas instâncias locais devem ter diretórios
separados para preservar saves/configurações. Consulte o
[roteiro de teste visual](../README.md#verificar-o-funcionamento).
Conectividade pública aprovada não equivale a validação visual da gameplay.
