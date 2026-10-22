# Contexto do projeto — leia ao retomar

Este arquivo apresenta as decisões estáveis. O estado operacional e as evidências
estão em [docs/WORK_SUMMARY.md](docs/WORK_SUMMARY.md). Ambos devem ser atualizados
quando houver mudanças relevantes; o Git é a referência para commits e tags.

## Objetivo e escopo

Criar uma base multiplayer para Pokémon Emerald no port nativo C/SDL2 de
`gradenGnostic/pokeemerald-multiplatform`, inicialmente no Linux. Jogadores devem
conectar por ENet e enxergar outros jogadores no mesmo mapa do overworld.
O objetivo imediato é uma build funcional; mudanças de gameplay, QoL, chat,
batalhas e trocas ficam para etapas futuras, conforme [ROADMAP.md](ROADMAP.md).

O repositório de trabalho é `tamydb/pokeemerald-mp`. Seu checkout original contém
a decompilação GBA, não o port SDL2. Por isso o port foi preparado separadamente
em `build/linux-port`, a partir de uma revisão fixa do upstream. Não substituir
a raiz pelo port nem pressupor que o Makefile GBA gera um executável de PC.

## Restrições definidas pelo usuário

- **Não publicar o jogo compilado no GitHub.** Publicar apenas fontes,
  ferramentas de recompilação, testes e documentação.
- ROM, saves, chaves, credenciais, builds e pacotes locais não entram no Git,
  em GitHub Releases ou em uploads de artefatos CI.
- Os pacotes em `build/releases` são exclusivamente locais. Tags representam
  checkpoints de fontes, não autorização de distribuição de binários.
- Documentar decisões, justificativas, alternativas relevantes e evidências.
  Não substituir evidência por suposição de funcionamento.
- Preparar testes para as features e salvar checkpoints após validação.
- Priorizar a base funcional antes de novas mecânicas ou QoL.

Política completa: [docs/distribution.md](docs/distribution.md).

## Arquitetura

- ENet 1.3.18 incorporado em `src/net/enet` e `include/net/enet`, licença preservada.
- Protocolo atual **2**, structs packed e inteiros multibyte em big endian.
- Canal 0: posições sem retransmissão; cliente envia a cada 50 ms no overworld.
- Canal 1: HELLO, WELCOME, JOIN e LEAVE confiáveis.
- JOIN estabelece os membros cujas posições são aceitas. LEAVE remove a permissão,
  impedindo que UDP atrasado recrie um jogador desconectado.
- Relay atribui IDs sem reutilização durante sua execução e repete snapshots.
- Sequência u32 usa aritmética serial para descartar duplicatas e pacotes antigos,
  incluindo rollover.
- O cliente altera Object Events apenas na thread existente do jogo. Polling
  com `enet_host_service(..., 0)` em CB2_Overworld, sem thread de rede adicional.
- O port já possuía uma thread SDL para AgbMain. A integração usa essa thread;
  shutdown ocorre nela, com sinalizador atômico e saída setjmp/longjmp no VBlank.
- Remotos usam IDs locais 240–247, sprite Brendan e reposicionamento por tile,
  somando MAP_OFFSET às coordenadas recebidas. Ainda não há interpolação.
- O servidor limita oito conexões totais; não executa gameplay nem carrega ROM/save.

Referências: [protocolo](docs/multiplayer.md), [servidor](docs/server.md),
[registro de decisões](docs/multiplayer-decisions.md).

## Builds e dependências externas

O port usa ponteiros de 32 bits: manter `-m32`. Não tentar migrar para 64 bits
como ajuste incidental. No Fedora 44, SDL2 é oferecido por `sdl2-compat-devel.i686`.
A ROM local apenas confirma a revisão suportada; o executável vem dos fontes.

Revisões fixas:

- Port SDL2: `db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46`.
- ENet: `2662c0de09e36f2a2030ccc2c528a3e4c9e8138a`.
- SHA-1 da ROM suportada: `f3ae088181bf583e55daf962a92bb46f4f1d07b7`.

`build/` é ignorado pelo Git. Em uma nova máquina, reconstruir o port com
`scripts/prepare_linux.py`, sem depender da existência da pasta atual.
Instruções: [docs/linux-playable.md](docs/linux-playable.md).

## Versionamento e retomada

Branch de implementação: `multiplayer/initial-linux`. Checkpoints:

- `v0.1.0-alpha.1`: base, servidor instalado e testes locais/na VM.
- `v0.1.0-alpha.2`: validação pública x86_64/i386; mesmo executável da alpha.1.

Commits posteriores podem atualizar documentação sem alterar VERSION ou criar
uma nova tag. Consultar `git status`, `git log` e as tags antes de modificar algo.
Preservar mudanças do usuário. Para experimentar checkpoints antigos, preferir
worktree/branch separado; não usar reset destrutivo.

Não houve push nesta sessão. Qualquer publicação futura deve respeitar a política
somente fontes/ferramentas e revisar o conteúdo preparado para envio.

## Próxima prioridade

A rede pública já passou nos testes automatizados. Falta verificar **dois jogadores
visíveis dentro do jogo**, mudança de mapa, saída, controles, áudio e save. Não
marcar o primeiro marco completo até registrar essas evidências. Consulte o
resumo operacional antes de repetir instalações ou alterar a VM.
