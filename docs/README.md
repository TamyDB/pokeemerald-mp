# Documentação do projeto

[Início](../README.md) · [Contexto](../CONTEXT.md) · [Roadmap](../ROADMAP.md) · [Changelog](../CHANGELOG.md)

Use este índice para encontrar instruções, entender decisões ou retomar o trabalho.
O [README principal](../README.md) é o ponto de partida para instalar e executar.

## Contexto e planejamento

| Documento | Finalidade |
| --- | --- |
| [Contexto](../CONTEXT.md) | Objetivo, restrições e decisões estáveis |
| [Roadmap](../ROADMAP.md) | Prioridades, status e critérios de aceite |
| [Resumo de trabalho](WORK_SUMMARY.md) | Estado da última validação, pendências e retomada |

## Guias do projeto

| Documento | Finalidade |
| --- | --- |
| [Linux](linux-playable.md) | Preparar, compilar, executar e validar a build |
| [Protocolo multiplayer](multiplayer.md) | Integração do cliente e contrato das mensagens |
| [Servidor](server.md) | Compilar, executar e operar o relay |
| [Distribuição e privacidade](distribution.md) | Conteúdo publicável e configuração privada |

## Trabalho e decisões

| Documento | Finalidade |
| --- | --- |
| [Decisões técnicas](multiplayer-decisions.md) | Problemas, justificativas, alternativas e evidências em ordem histórica |
| [Registro de 21/10/2026](history/2026-10-21.md) | Fotografia completa da primeira sessão de implementação |

## Changelog e versionamento

| Documento | Finalidade |
| --- | --- |
| [Changelog](../CHANGELOG.md) | Mudanças e limites de cada checkpoint |
| [Versionamento](versioning.md) | Relação entre VERSION, commits, tags e pacotes locais |
| [VERSION](../VERSION) | Versão declarada do projeto, lida pelas ferramentas |

## Referências herdadas

Esses documentos tratam da decompilação GBA e de suas ferramentas. Não representam
suporte validado ao multiplayer nessas plataformas.

| Documento | Escopo |
| --- | --- |
| [Instalação GBA](../INSTALL.md) | Instruções upstream para gerar uma ROM |
| [WSL1 legado](legacy_WSL1_INSTALL.md) | Complemento histórico da instalação GBA |
| [wav2agb](../tools/wav2agb/README.md) | Conversão de áudio para formatos do GBA |

## Como manter a documentação

- Mudou a forma de executar? Atualize o README e o guia correspondente.
- Mudou um requisito ou uma restrição estável? Atualize CONTEXT.md.
- Mudou o escopo ou o aceite de uma entrega? Atualize ROADMAP.md.
- Produziu uma evidência ou encontrou uma limitação? Atualize WORK_SUMMARY.md.
- Tomou uma decisão técnica? Registre problema, decisão, motivo e validação no registro de decisões.
- Preparou um checkpoint? Atualize CHANGELOG.md e siga o guia de versionamento.

Mantenha histórico datado em `docs/history/` quando precisar preservar uma passagem
de contexto completa. O resumo atual deve mostrar o estado consolidado, sem exigir
que o leitor reconstrua decisões lendo adendos. Use links relativos, exemplos sem
endpoint privado e comandos executados a partir da raiz, salvo indicação explícita.
