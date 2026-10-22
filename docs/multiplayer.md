# Protocolo e integração multiplayer

[Início](../README.md) · [Documentação](README.md) · [Servidor](server.md) · [Decisões](multiplayer-decisions.md)

Este checkout contém o jogo GBA, sem o port SDL2. `Makefile_pc` compila
uma biblioteca estática da camada de rede, não um executável do jogo.
O Makefile GBA exclui `src/net` para não incorporar sockets à ROM.

## Navegação

- [Compilar e conectar](#compilar-e-conectar)
- [Contrato do servidor](#contrato-do-servidor-protocolo-2)
- [Verificação](#verificação)
- [Dependência](#dependência)

## Compilar e conectar

Execute `make -f Makefile_pc network`. O alvo produz
`build/pc-net/libpokeemerald-net.a` e gera os cabeçalhos de mapas necessários.
Para Windows use um compilador MinGW e `PLATFORM=windows`; para Android use
Clang do NDK e `PLATFORM=android`. Windows e Android ainda não foram testados.
As opções de sockets estão em `LDLIBS` e precisam ser usadas no link final.

A integração Linux reproduzível é aplicada por `scripts/prepare_linux.py`; siga
o [guia Linux](linux-playable.md) para usá-la. O hook usa
`InitNetworkClientFromEnvironment` na primeira entrada no overworld.

Para integrar outro port manualmente, compile também o jogo com `-DMULTIPLAYER` e vincule a biblioteca.
Na thread do jogo, após carregar o save, chame
`InitNetworkClient(enderecoConfigurado, portaConfigurada)`. O retorno indica início da tentativa;
o handshake termina de forma assíncrona. Endereços são IPv4 numéricos para
não bloquear com DNS. `CB2_Overworld` chama `UpdateNetworkClient`.
Chame `ShutdownNetworkClient` antes de descarregar o save ou encerrar o jogo.
Durante menus/batalhas o polling fica suspenso; ao retornar ao overworld o
ENet pode detectar timeout. Não há reconexão automática nesta etapa.

## Contrato do servidor (protocolo 2)

Todos os inteiros multibyte trafegam em big endian; os pacotes são packed.
Os tipos e tamanhos estão em `include/net/net_client.h`.

1. Cliente conecta com dois canais e connect data `NET_PROTOCOL_VERSION`.
2. No canal 1 envia `NetHelloPacket`; servidor valida a versão e responde
   `NetWelcomePacket` com um ID único para aquela sessão.
3. Servidor anuncia membros com `NetJoinPacket` confiável; cliente só aceita
   posições de membros presentes. Cliente envia `PositionPacket` no canal 0 a cada 50 ms no overworld.
4. Servidor retransmite `NetRemotePositionPacket`, com o ID do remetente
   obtido da sessão e o pacote original, aos demais clientes no canal 0.
5. Ao sair um jogador, servidor envia `NetLeavePacket` no canal 1.

O servidor deve manter IDs únicos durante a sessão, enviar o último estado
a novos participantes e impedir que posições antigas atravessem um evento
de saída. Os dois canais têm ordenação independente. O servidor agora está em
`server/main.c`; compilação e operação em [server.md](server.md).

O cliente possui até oito slots remotos, limitados também pelos Object Events
livres. O relay aceita oito conexões totais, incluindo o jogador local.
IDs locais 240–247 são reservados; conflitos existentes não são sobrescritos.
A posição usa coordenadas do save, somando `MAP_OFFSET` apenas ao aplicar
nos Object Events. Sprites usam Brendan e são reposicionados por tile;
`animState` está reservado para futura interpolação/animação de caminhada.
Colisão e interação seguem as regras normais dos Object Events do jogo.
Chat, batalha, troca e UI de conexão ainda não estão implementados; o canal
confiável já é usado para handshake e saída.

## Verificação

```bash
make -f Makefile_pc network
make -f Makefile_pc test-network
```

O teste de sincronização usa APIs de Object Events simuladas. Valida sequência,
rollover, troca de mapa e preservação de NPCs. Não substitui testes no jogo.

## Dependência

ENet 1.3.18, commit `2662c0de09e36f2a2030ccc2c528a3e4c9e8138a`,
obtido do [repositório ENet](https://github.com/lsalzman/enet). Fontes e headers sem alterações;
licença MIT preservada em `src/net/enet/LICENSE`.

As decisões técnicas e a cobertura dos testes são registradas em
[multiplayer-decisions.md](multiplayer-decisions.md).
