# Roadmap

[Início](README.md) · [Documentação](docs/README.md) · [Changelog](CHANGELOG.md) · [Resumo de trabalho](docs/WORK_SUMMARY.md)

Objetivo atual: produzir uma build Linux reproduzível com conexão ENet e jogadores
visíveis no mesmo mapa. Novas mecânicas e QoL entram depois dessa base.

**Revisão documental:** 22/10/2026 · **Últimas evidências:** 21/10/2026.
Registro técnico: [decisões](docs/multiplayer-decisions.md).

## Navegação

- [Base funcional Linux e servidor](#marco-1--base-funcional-linux-e-servidor)
- [Estabilidade das sessões](#marco-2--estabilidade-das-sessões)
- [Possíveis features multiplayer](#possíveis-features-multiplayer)
- [Melhorias de qualidade de vida](#possíveis-melhorias-de-qol)
- [Detalhamento das propostas](#detalhamento-das-propostas)
- [Critério para salvar checkpoints](#critério-para-salvar-checkpoints)

As propostas não têm compromisso de prazo. O Marco 1 permanece aberto até a
validação visual e os testes manuais indicados abaixo.

## Como ler os status

- **Validado:** há evidência automatizada ou operacional descrita na linha.
- **Implementado:** código disponível, mas falta a validação indicada.
- **Em andamento:** entrega ainda sendo construída.
- **Bloqueado:** depende de uma ação externa identificada.
- **Proposto:** ideia de backlog; ainda sem compromisso de implementação.

## Marco 1 — Base funcional Linux e servidor

| Feature | Status | Evidência / próximo passo |
| --- | --- | --- |
| ENet incorporado e protocolo versionado | Validado | Compilação Linux e teste UDP local nos dois canais |
| Polling na thread existente do jogo | Implementado | Hook em CB2_Overworld; falta teste de sessão prolongada no jogo |
| Handshake e IDs de jogador | Validado | Testes com cliente real e servidor local/na VM |
| Entrada e saída confiáveis | Validado | JOIN/LEAVE; posições atrasadas não recriam membros ausentes |
| Posição enviada a 20 Hz e ordem de sequência | Validado | Testes de intervalo, serialização, duplicatas e rollover |
| Relay de posições, entrada tardia e reconexão | Validado | Integração com três clientes em localhost e dentro da VM |
| Objetos remotos no mesmo mapa | Implementado | Testes com APIs de sprites simuladas; falta verificar dois jogadores no jogo |
| Preservação de NPCs ao reutilizar slots | Validado | Teste de sincronização com slot reutilizado |
| Executável Linux nativo de 32 bits | Validado | ELF compilado; inicialização sem janela chegou ao quadro 300 |
| Imagem, áudio, controles e save | Implementado | Recursos do port upstream; falta teste manual desta build |
| Serviço ENet na Oracle | Validado | systemd ativo e porta UDP localmente configurada em escuta; reinício do serviço e novo teste de integração aprovados |
| Regra UDP configurada no firewall da VM | Validado | Regra específica ativa e serviço para reaplicar após boot |
| Conexão pela internet | Validado | Integração pública com três clientes aprovada em x86_64 e i386 após liberar a porta configurada na OCI |
| Empacotamento Linux | Validado | Pacote alpha.1 gerado e manifesto verificado; alpha.2 registra a validação pública, sem mudança no executável |
| Checkpoints de fontes | Validado | alpha.1: base; alpha.2: rede pública; alpha.3: configuração privada. Gameplay visual pendente |
| Configuração privada de IPv4 e porta | Validado | Arquivo local/ambiente, validação e modelos públicos vazios na alpha.3 |

### Critérios para concluir o marco

- [x] Compilar cliente e servidor a partir de fontes identificadas.
- [x] Passar testes determinísticos e integração com transporte real.
- [x] Instalar servidor como serviço com reinicialização automática configurada.
- [x] Confirmar conexão e troca de posições pela internet.
- [ ] Abrir duas instâncias, entrar no mesmo mapa e observar o outro jogador.
- [ ] Verificar mudança de mapa, saída de um jogador e ausência de NPCs corrompidos.
- [ ] Verificar controles, áudio e persistência do save no jogo.
- [x] Registrar checkpoint e instruções de recuperação.

O teste sem janela comprova inicialização; não comprova toda a gameplay.
A rede pública foi testada com clientes reais sem renderização; ainda falta a
validação visual de dois jogadores no jogo.

## Marco 2 — Estabilidade das sessões

| Feature | Status | Resultado esperado |
| --- | --- | --- |
| Status de conexão visível | Proposto | Jogador entende quando conectou, perdeu conexão ou está offline |
| Reconexão automática | Proposto | Recuperar sessão sem reiniciar o jogo |
| Polling durante menus e batalhas | Proposto | Manter conexão sem modificar o overworld fora de seu contexto |
| Animação e interpolação dos remotos | Proposto | Caminhada mais suave, com orientação e animações coerentes |
| Identidade visual e nome do jogador | Proposto | Distinguir participantes no mapa |
| Testes de perda, atraso e reordenação de pacotes | Proposto | Medir o comportamento em redes instáveis |
| Testes de carga com oito jogadores | Proposto | Confirmar capacidade e uso de recursos da VM |
| Colisão e interação dos objetos remotos | Proposto | Definir comportamento para evitar bloqueio de passagens e scripts acidentais |
| Compatibilidade entre versões | Proposto | Recusar versões incompatíveis com mensagem compreensível |

## Possíveis features multiplayer

| Feature | Status | Observação |
| --- | --- | --- |
| Chat | Proposto | Mensagens no canal confiável; interface e limites a definir |
| Convites de batalha | Proposto | Definir consentimento, estado da sessão e recuperação de interrupções |
| Trocas de Pokémon | Proposto | Definir consistência e recuperação antes de alterar saves |
| Salas privadas e convite | Proposto | Separar grupos de jogadores e controlar entrada |
| Eventos cooperativos | Proposto | Definir quais eventos são compartilhados e quais permanecem individuais |
| Atividades e desafios em grupo | Proposto | Avaliar após estabilizar a sincronização básica |

## Possíveis melhorias de QoL

| Feature | Status | Observação |
| --- | --- | --- |
| Menu multiplayer com servidor personalizado | Proposto | Cada jogador informa IP e porta do servidor; conectar, desconectar e lembrar a configuração, sem depender de um endereço fixo |
| Remapear controles | Proposto | Avaliar recursos existentes do port antes de implementar |
| Indicadores de mapa e presença dos amigos | Proposto | Facilitar encontros sem alterar a progressão |
| Preferências de áudio e interface | Proposto | Avaliar o que o port já oferece e validar nesta build |
| Novas opções de configuração | Proposto | Levantar opções desejadas, verificar as já existentes e definir persistência e valores padrão antes de implementar |
| Nomes próprios com inicial maiúscula | Proposto | Substituir caixa alta nos textos exibidos, por exemplo BRENDAN → Brendan; revisar nomes compostos, exceções e nomes definidos pelo jogador |
| Backup de saves e perfis separados | Proposto | Facilitar testes e recuperação do progresso |
| Distribuição Linux mais simples | Proposto | Avaliar pacote portátil após estabilizar dependências |
| Android e Windows com multiplayer | Proposto | Exigem compilação e testes próprios; ainda não validados |

## Detalhamento das propostas

### Menu multiplayer

- [ ] Adicionar uma opção Multiplayer ao menu do jogo.
- [ ] Permitir que cada jogador informe o IP e a porta do servidor desejado.
- [ ] Validar endereço e porta antes de tentar conectar, mostrando erros compreensíveis.
- [ ] Oferecer ações de conectar e desconectar, com status visível da conexão.
- [ ] Salvar IP e porta nas preferências locais do jogador e permitir editá-los.
- [x] Permitir IP e porta via arquivo local ignorado pelo Git; menu no jogo ainda pendente.
- [ ] Testar conexão com servidores e portas diferentes, configurações inválidas
  e persistência das preferências.

Decisão do usuário: nesta fase, usar a VM atual para testes com amigos. Na versão
futura, cada jogador deverá informar IP e porta do servidor pelo menu; a VM e
seu endereço não serão uma dependência fixa do projeto. O menu ainda não está
implementado nesta etapa; o launcher lê IP e porta de um arquivo local
preenchido pelo jogador, sem endpoint padrão publicado.

### Configurações e textos

- [ ] Definir com o usuário quais novas opções entrarão no menu de configurações.
- [ ] Auditar configurações existentes do port para evitar duplicação.
- [ ] Definir valores padrão, persistência por perfil e comportamento ao restaurar padrões.
- [ ] Levantar ocorrências de nomes próprios em caixa alta nos textos do jogo.
- [ ] Padronizar a exibição para inicial maiúscula e demais letras minúsculas,
  revisando nomes compostos e exceções antes de aplicar conversão automática.
- [ ] Preservar identificadores internos, códigos de controle e a grafia escolhida
  pelo jogador para nomes/apelidos; não alterar saves sem uma decisão específica.
- [ ] Verificar diálogos, menus e quebras de linha após as alterações de texto.

Esses itens foram solicitados para uma etapa futura e ainda não foram implementados.

## Critério para salvar checkpoints

Uma entrega só passa a validada quando houver evidências registradas e limites
explícitos. Atualize os itens correspondentes, execute os testes pertinentes e
registre o resultado no [resumo de trabalho](docs/WORK_SUMMARY.md).

Mudanças por versão ficam no [changelog](CHANGELOG.md). O procedimento para
VERSION, commits, tags, worktrees e pacotes locais está no
[guia de versionamento](docs/versioning.md). Tags não autorizam distribuição do
jogo compilado nem concluem automaticamente as pendências deste roadmap.
