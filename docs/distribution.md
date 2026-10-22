# Distribuição e privacidade

[Início](../README.md) · [Documentação](README.md) · [Versionamento](versioning.md)

O projeto disponibiliza fontes, ferramentas de recompilação, testes e documentação.
Por determinação do usuário, **o jogo compilado não deve ser enviado ao GitHub**.
Cada pessoa prepara sua build localmente com os arquivos necessários.

## O que pode ser publicado

| Conteúdo | Destino permitido nesta política |
| --- | --- |
| Fontes, scripts, testes e documentação revisados | Repositório de fontes |
| Tags de checkpoint | Referências aos fontes, com pendências e limites documentados |
| Fontes e executável do relay sem conteúdo do jogo | Servidor de testes |
| Jogo compilado e pacote de teste | Apenas armazenamento e execução locais |
| ROM, save, credenciais e endpoint privado | Arquivos locais; fora do conteúdo publicável |

## Artefatos do jogo

- Não incluir jogo compilado, ROM fornecida, saves ou pacotes locais em commits,
  push, GitHub Releases ou uploads de artefatos CI.
- Tags identificam checkpoints de fontes; não autorizam publicar binários.
- `scripts/package_linux.py` prepara somente um pacote local para testes.
- `build/`, `dist/`, `releases/`, executáveis e pacotes estão excluídos do Git.
- Revisar o conteúdo preparado antes de qualquer publicação.

Na auditoria de 21/10/2026, nenhum executável ou pacote do jogo havia sido enviado
ao GitHub; o CI inspecionado não publicava ROM/jogo como artefato ou release.
Essa evidência não substitui a revisão de mudanças futuras. A política não altera
arquivos históricos herdados do upstream.

## Endpoint privado

IP e porta reais da VM não devem aparecer em documentação, commits, tags ou
exemplos publicáveis. O jogador usa `config/multiplayer.env`, ignorado pelo Git,
ou variáveis de ambiente. O modelo versionado tem campos vazios, sem servidor
ou porta padrão. Templates de deploy recebem a porta por arquivo local.

Exemplos de loopback e portas de desenvolvimento são ilustrativos; não devem
ser substituídos pelos valores privados do ambiente de testes nos documentos.

## Histórico de publicação

O histórico local de implementação foi sanitizado antes da primeira publicação,
removendo os valores privados dos checkpoints anteriores. Branch de implementação
e três tags alpha sanitizadas foram enviados ao GitHub com autorização, sem
force-push. Pacotes antigos continuam exclusivamente locais e não podem ser publicados.

Não refazer a sanitização nem mover tags publicadas para ajustes documentais.
Consulte o [guia de versionamento](versioning.md) para interpretar hashes antigos.
