# Servidor ENet mínimo

O servidor em `server/main.c` implementa apenas presença e posições de até oito
conexões. Não carrega ROM, save, scripts de mapa ou dados de Pokémon. O limite é
de oito conexões totais; o cliente pode representar até oito remotos, condicionado
aos slots de Object Events disponíveis. Servidor ainda é experimental.

## Protocolo 2

Os tipos são definidos em `include/net/net_client.h`; inteiros multibyte em big
endian. O connect data do ENet e o HELLO precisam informar a versão 2.

1. Cliente envia HELLO confiável no canal 1.
2. Servidor envia WELCOME com ID exclusivo daquela execução do servidor.
3. Servidor envia JOINs confiáveis para estabelecer a lista de participantes.
4. Cliente envia POSITION no canal 0. Servidor valida campos e sequência,
   limita a 40 posições aceitas por segundo por conexão e encaminha aos demais.
5. Servidor repete snapshots a cada 50 ms para recuperar o estado de entradas
   tardias e de posições que tenham chegado antes do JOIN.
6. Servidor envia LEAVE confiável na saída. Cliente remove o membro e ignora
   posições seguintes daquele ID, até existir um novo JOIN válido.

A versão 1 não tinha controle de entrada confiável; por isso clientes antigos
não são compatíveis. IDs não são reutilizados na mesma execução do servidor.
São reservados pelo servidor, nunca escolhidos pelo remetente da posição.
Clientes sem handshake são removidos após 10 segundos.

O relay não valida progressão ou colisão nem fornece autenticação de jogadores.
Chat, batalhas e trocas não estão implementados.

## Compilar e testar

```sh
make -f Makefile_pc server relay-test test-network
python3 scripts/test_relay.py
```

A integração usa o cliente real do jogo com callbacks de sincronização simulados
e dois probes ENet adicionais. Valida handshake, IDs, posições, pacotes antigos,
campos inválidos, canal incorreto, entrada tardia, saída e reconexão.
O teste requer permissão para sockets UDP. Não verifica sprites na tela.

Para testar um servidor existente (cria até três conexões temporárias):

```sh
build/pc-net/relay-test ${POKE_NET_SERVER} ${POKE_NET_PORT}
```

## Instalação atual

VM `tet`, usuário `ubuntu`, Ubuntu 24.04 x86_64. Fontes em
`/home/ubuntu/pokeemerald-server`; binário em `build/pc-net/pokeemerald-server`
dentro dessa pasta. gcc, make e libc6-dev foram instalados via apt.

Serviço do usuário: `deploy/pokeemerald-server.service`. Linger habilitado para
iniciar sem login interativo. Regra local da porta reaplicada pelo serviço de
sistema `deploy/pokeemerald-udp.service`. As regras existentes não foram removidas.

```sh
ssh tet 'systemctl --user status pokeemerald-server'
ssh tet 'journalctl --user -u pokeemerald-server -n 50 --no-pager'
ssh tet 'systemctl --user restart pokeemerald-server'
```

Para parar o relay: `systemctl --user stop pokeemerald-server` na VM.
Para desabilitar: `systemctl --user disable --now pokeemerald-server`.
Para retirar a liberação local: `sudo systemctl disable --now pokeemerald-udp`.

## Rede Oracle

O teste dentro da VM passou. O primeiro teste público expirou e o contador da
regra UDP ${POKE_NET_PORT} permaneceu em zero. A hipótese é bloqueio na rede anterior à VM;
não foi comprovada conectividade pública.

Na Security List da subnet ou NSG da VNIC, permitir entrada stateful:

| Campo | Valor |
| --- | --- |
| Origem CIDR | 0.0.0.0/0 |
| Protocolo | UDP |
| Porta de origem | Todas |
| Porta de destino | ${POKE_NET_PORT} |

Referência: [documentação de Security Lists da Oracle](https://docs.oracle.com/en-us/iaas/Content/Network/Concepts/creating-securitylist.htm).
Após aplicar, repetir o teste de integração público. Não é necessário abrir TCP
${POKE_NET_PORT}. Não foi acessado o painel/API OCI nesta entrega.

## Jogar

```sh
./scripts/play_multiplayer_linux.sh
```

O launcher configura o endereço da VM e mantém porta ${POKE_NET_PORT}. A conexão começa ao
entrar no overworld. Duas instâncias locais precisam de diretórios de trabalho
separados para não compartilharem save/config simultaneamente.
Para apontar a outro IPv4: `POKE_NET_SERVER=127.0.0.1 ./scripts/play_multiplayer_linux.sh`.
