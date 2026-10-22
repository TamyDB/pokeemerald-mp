# Distribuição: somente ferramentas e fontes

Regra definida pelo usuário: o jogo compilado não deve ser enviado ao GitHub.
O repositório disponibiliza fontes, scripts de recompilação, testes e documentação;
cada pessoa gera sua build localmente com os arquivos necessários.

- Não fazer commit, push, GitHub Release ou upload de artefato CI contendo o jogo
  compilado, ROM fornecida pelo usuário, saves ou pacotes locais do jogo.
- As tags identificam checkpoints dos fontes; não autorizam publicar binários.
- `scripts/package_linux.py` prepara um pacote estritamente local para testes.
- `build/`, `dist/`, `releases/`, executáveis e pacotes estão excluídos do Git.
- Antes de qualquer publicação, revisar os arquivos preparados para envio.
- A VM recebe apenas os fontes e o executável do relay, que não carrega conteúdo
  do jogo. A ROM e os pacotes do jogo permanecem no computador local.

Nenhum executável ou pacote do jogo foi enviado ao GitHub nesta sessão.
O workflow CI existente não publica a ROM/executável como artefato ou release.
Esta regra não altera os arquivos históricos do upstream.
