# Changelog

[Início](README.md) · [Documentação](docs/README.md) · [Versionamento](docs/versioning.md) · [Roadmap](ROADMAP.md)

Histórico dos checkpoints da camada multiplayer deste projeto. As entradas iniciais
foram consolidadas em 22/10/2026 a partir das tags e dos registros de validação.
O histórico anterior da decompilação permanece no Git.

**Tags identificam fontes, não binários disponíveis para download.** Todos os
checkpoints abaixo continuam experimentais; a validação visual de dois jogadores
no jogo permanece pendente.

## Não lançado

### Documentação

- README reorganizado com apresentação, instalação, arquitetura e objetivos.
- Índice da documentação e separação entre contexto, estado atual e histórico de trabalho.
- Changelog e guia de versionamento adicionados.
- Guias técnicos revisados e caminhos pessoais substituídos por exemplos genéricos.

Esta seção registra alterações em preparação, posteriores à alpha.3. Não altera
VERSION, não representa uma nova tag e não acrescenta validação de gameplay.

## v0.1.0-alpha.3 — 2026-10-21

### Adicionado e alterado

- Configuração privada de IPv4 e porta por arquivo local ou ambiente, sem endpoint padrão.
- Validação dos campos sem executar código do arquivo de configuração.
- Porta explícita no relay e templates de deploy parametrizados.
- Modelos públicos vazios e exclusão das configurações pessoais do Git.
- Testes de configuração, precedência, normalização e ausência de execução de comandos.

### Validação registrada

Testes de configuração e cliente, integração com porta efêmera, recompilação Linux
e smoke test offline aprovados. O menu Multiplayer ainda não foi implementado.

O histórico foi sanitizado antes da primeira publicação. Branch e tags sanitizados
foram publicados posteriormente, com autorização e sem builds ou endpoint privado.

## v0.1.0-alpha.2 — 2026-10-21

### Validação registrada

- Integração pela internet aprovada com clientes x86_64 e i386 após liberação UDP na OCI.
- Handshake, presença, posições, descarte de pacotes antigos/inválidos, entrada tardia,
  saída e reconexão verificados com cliente real e probes ENet.

Checkpoint de documentação, status e VERSION: mesmo executável e protocolo da
alpha.1. Os callbacks de sprites desses testes são simulados.

## v0.1.0-alpha.1 — 2026-10-21

### Adicionado

- ENet 1.3.18 incorporado, cliente e relay C para até oito conexões.
- Protocolo 2 com handshake, presença confiável e posições a cada 50 ms.
- Sincronização de objetos remotos no overworld e proteção de NPCs locais.
- Preparação reproduzível do port SDL2 Linux i386 e scripts de execução/teste.
- Serviços do relay e firewall na VM, empacotamento exclusivamente local e roadmap.

### Corrigido

- Integração de includes e bibliotecas no Fedora multilib.
- Encerramento da thread do jogo, validado sem necessidade de SIGKILL na nova build.

### Validação registrada

Testes determinísticos, UDP loopback, integração local/na VM e inicialização sem
janela aprovados. A rede pública ainda estava pendente neste checkpoint; passou
na alpha.2. Renderização multiplayer, áudio, controles e save não foram validados.

## Evidências e referências

- [Resumo atual](docs/WORK_SUMMARY.md): resultados e limites consolidados.
- [Decisões técnicas](docs/multiplayer-decisions.md): contexto de cada alteração.
- [Registro de 21/10/2026](docs/history/2026-10-21.md): passagem de contexto original.

Consulte `git show NOME_DA_TAG` para obter o commit publicado. Hashes de manifestos
locais anteriores à sanitização podem divergir do histórico compartilhado.
