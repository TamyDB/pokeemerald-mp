# Versionamento e checkpoints

[Início](../README.md) · [Documentação](README.md) · [Changelog](../CHANGELOG.md)

Este guia explica como interpretar e preparar checkpoints. Não cria autorização
para publicar o jogo compilado; a [política de distribuição](distribution.md)
continua válida.

## Fontes de referência

| Referência | O que informa |
| --- | --- |
| `VERSION` | Versão declarada pelas ferramentas, atualmente `0.1.0-alpha.3` |
| Tag `v0.1.0-alpha.N` | Commit exato de um checkpoint de fontes |
| `CHANGELOG.md` | Mudanças, validações e limites por checkpoint |
| `ROADMAP.md` | Critérios de conclusão e propostas futuras |
| `docs/WORK_SUMMARY.md` | Último estado operacional documentado |
| Git | Histórico efetivo de commits, branches e tags |

A linha atual usa `0.1.0-alpha.N`, com prefixo `v` nas tags. O protocolo de rede
é versionado separadamente: está na versão 2. Não presumir compatibilidade de
rede apenas porque dois clientes pertencem à mesma série alpha.

Commits de documentação podem vir depois de uma tag sem alterar VERSION. Portanto,
VERSION não garante que HEAD corresponda exatamente à tag daquele número.

## Conferir o checkout

Na raiz do projeto:

```bash
git status --short
git branch --show-current
cat VERSION
git log -5 --oneline
git tag --list 'v*'
git describe --tags --always
```

Os checkpoints existentes são alpha.1 (base Linux/relay), alpha.2 (rede pública
validada) e alpha.3 (configuração privada). Consulte o changelog para seus limites.
O branch de implementação publicado é `multiplayer/initial-linux`.

## Preparar um novo checkpoint

1. Defina a entrega e seus critérios de aceite no roadmap.
2. Implemente e execute os testes pertinentes. Registre resultado observado e limites.
3. Atualize os guias afetados, o resumo de trabalho e as decisões técnicas.
4. Consolide as mudanças da seção “Não lançado” do changelog na nova entrada datada.
5. Atualize VERSION quando houver um novo checkpoint de versão.
6. Revise o diff e os arquivos preparados: sem ROM, build, save, credenciais ou endpoint privado.
7. Salve um commit de escopo claro e crie uma nova tag quando a validação prevista estiver registrada.

Uma tag não torna pendências concluídas. Não mover tags publicadas nem reescrever
histórico compartilhado para ajustes rotineiros. A sanitização de privacidade foi
concluída antes da primeira publicação; hashes locais anteriores podem divergir.

## Consultar uma versão sem perder trabalho

Para inspecionar as fontes:

```bash
git show v0.1.0-alpha.3
```

Para experimentar em um checkout separado:

```bash
git worktree add ../pokeemerald-checkpoint v0.1.0-alpha.3
```

Escolha outro destino se a pasta já existir. O novo worktree não contém a build,
a ROM nem a configuração privada do checkout original. Prepare e compile o port
nele conforme o [guia Linux](linux-playable.md); mantenha saves e configurações
separados. Não usar reset destrutivo para testar checkpoints antigos.

## Empacotar localmente

O empacotador exige:

- Executável Linux ELF32 já compilado e fontes de rede sincronizados com o port.
- Checkout limpo, incluindo ausência de arquivos novos não ignorados.
- Tag existente apontando exatamente para HEAD.
- Diretório de saída ainda inexistente.

Em um checkout preparado na tag alpha.3, por exemplo:

```bash
python3 scripts/package_linux.py v0.1.0-alpha.3
```

A saída fica em `build/releases/`, com arte, launcher, licenças e manifesto
SHA-256 associado ao commit e à revisão do port. O pacote não inclui ROM, save
ou bibliotecas do sistema. Ele é **exclusivamente local**, sem upload ao GitHub,
Releases ou artefatos CI. Não reutilize manifestos antigos como referência do
histórico sanitizado.

## Publicação de fontes

Antes de um envio autorizado, confira commits, tags e conteúdo preparado.
Publicar somente fontes, ferramentas, testes e documentação, preservando
configurações privadas. A publicação anterior do branch e das três tags não é
uma etapa a repetir para organizar documentos.
