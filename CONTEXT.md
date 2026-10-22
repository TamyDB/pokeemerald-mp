# Contexto do projeto

[Início](README.md) · [Documentação](docs/README.md) · [Roadmap](ROADMAP.md) · [Retomar o trabalho](docs/WORK_SUMMARY.md)

Este documento reúne decisões estáveis. O estado operacional e as evidências
ficam no [resumo de trabalho](docs/WORK_SUMMARY.md); mudanças por versão, no
[changelog](CHANGELOG.md). Consulte o Git para confirmar branch, commits e tags.

## Objetivo e escopo

Criar uma base multiplayer para Pokémon Emerald no port nativo C/SDL2 de
[gradenGnostic/pokeemerald-multiplatform](https://github.com/gradenGnostic/pokeemerald-multiplatform),
inicialmente no Linux. Jogadores devem conectar por ENet e enxergar outros
jogadores no mesmo mapa do overworld.

A prioridade é uma build funcional e reproduzível. Chat, batalhas, trocas e
melhorias de qualidade de vida dependem da validação dessa base e estão no roadmap.

## Restrições acordadas

| Regra | Aplicação |
| --- | --- |
| Publicar somente fontes, ferramentas, testes e documentação | Não enviar ROM, jogo compilado, saves ou pacotes locais ao GitHub, Releases ou artefatos CI |
| Preservar a privacidade do endpoint | IP e porta reais ficam em configuração local ignorada; modelos públicos têm campos vazios |
| Registrar evidências | Distinguir implementação, resultado observado e validação pendente |
| Validar antes de fechar marcos | Preparar testes pertinentes e salvar checkpoints após validação |
| Preservar o trabalho existente | Não substituir a raiz pelo port nem usar reset destrutivo para experimentar versões |

Política completa: [distribuição e privacidade](docs/distribution.md).

## Organização e build

O repositório de trabalho é `TamyDB/pokeemerald-mp`. A raiz contém a decompilação
GBA, não o port SDL2. O port é preparado separadamente em `build/linux-port`, a
partir de uma revisão fixa. `build/` é ignorado pelo Git e precisa ser recriado
em uma nova máquina por `scripts/prepare_linux.py`.

O `Makefile_pc` da raiz compila rede, servidor e testes. O jogo Linux completo é
compilado por `scripts/build_linux.sh` no port preparado. O port exige ponteiros
de 32 bits: manter `-m32`; não migrar para 64 bits como ajuste incidental.
No Fedora 44, a API SDL2 é fornecida por `sdl2-compat-devel.i686`.

| Referência fixa | Revisão |
| --- | --- |
| Port SDL2 | `db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46` |
| ENet 1.3.18 | `2662c0de09e36f2a2030ccc2c528a3e4c9e8138a` |
| SHA-1 da ROM suportada | `f3ae088181bf583e55daf962a92bb46f4f1d07b7` |

A ROM local apenas confirma a revisão suportada; o executável vem dos fontes.
Preparação e dependências: [guia Linux](docs/linux-playable.md).

## Arquitetura acordada

| Componente | Contrato |
| --- | --- |
| Protocolo | Versão 2, structs packed, inteiros multibyte em big endian |
| Canal 0 | Posições sem retransmissão, enviadas a cada 50 ms no overworld |
| Canal 1 | HELLO, WELCOME, JOIN e LEAVE confiáveis |
| Presença | JOIN autoriza posições; LEAVE remove a permissão, impedindo recriação por UDP atrasado |
| Relay | Até oito conexões totais, IDs sem reutilização durante a execução e snapshots repetidos; sem gameplay, ROM ou save |
| Sequência | u32 com aritmética serial para descartar duplicatas e estado antigo, inclusive rollover |
| Thread do jogo | Polling não bloqueante em CB2_Overworld; Object Events alterados na thread existente |
| Encerramento SDL | Sinalizador atômico e saída setjmp/longjmp no VBlank da thread do jogo |
| Remotos | IDs locais 240–247, sprite Brendan, posição por tile com MAP_OFFSET; sem interpolação |

ENet é incorporado em `src/net/enet` e `include/net/enet`, com licença preservada.
O port já possui uma thread SDL para AgbMain; a integração não cria outra thread
de rede. Detalhes: [protocolo](docs/multiplayer.md), [servidor](docs/server.md) e
[decisões técnicas](docs/multiplayer-decisions.md).

## Conexão e direção do produto

A VM atual atende aos testes iniciais com amigos. Seu endereço não deve se tornar
uma dependência fixa do produto. O launcher exige IPv4 e porta em
`config/multiplayer.env`, local e ignorado pelo Git, ou nas variáveis de ambiente.
O menu Multiplayer para editar, conectar, desconectar e lembrar preferências é
uma proposta de backlog.

## Checkpoints e publicação

Branch de implementação: `multiplayer/initial-linux`. As tags alpha.1, alpha.2 e
alpha.3 foram publicadas no GitHub após autorização e auditoria do histórico
sanitizado; o master foi preservado. Não refazer a sanitização nem mover tags
publicadas sem necessidade. Novas publicações devem seguir a política somente fontes.

A versão declarada está em [VERSION](VERSION). Commits de documentação podem
suceder uma tag sem alterar a versão. Consulte [CHANGELOG.md](CHANGELOG.md) e o
[guia de versionamento](docs/versioning.md) para interpretar os checkpoints.

## Prioridade de retomada

A rede pública passou nos testes automatizados. Falta verificar **dois jogadores
visíveis no jogo**, movimento, mudança de mapa, saída, NPCs, controles, áudio e
save. Não marcar o primeiro marco completo sem registrar essas evidências.
Antes de repetir instalações ou alterar a VM, leia o [resumo atual](docs/WORK_SUMMARY.md).
