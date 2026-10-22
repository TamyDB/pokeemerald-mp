# Decisões técnicas e validação

[Início](../README.md) · [Documentação](README.md) · [Resumo atual](WORK_SUMMARY.md) · [Changelog](../CHANGELOG.md)

Este documento registra contexto, decisões técnicas, justificativas e evidências.
Deve ser atualizado junto com alterações de comportamento e novos testes.
As instruções de uso e o contrato do servidor estão em [multiplayer.md](multiplayer.md).

> **Leitura histórica:** cada entrada descreve o estado observado naquele momento.
> Bloqueios iniciais de dependências, servidor e rede pública foram resolvidos nas
> entradas seguintes. O estado consolidado está no resumo atual; a validação visual
> de dois jogadores permanece pendente.

## Navegação

- [Integração inicial](#2026-10-21--integração-inicial)
- [Preparação Linux](#2026-10-21--preparação-da-versão-linux)
- [Servidor e primeiro checkpoint](#2026-10-21--servidor-mínimo-e-primeiro-checkpoint)
- [Conectividade pública — alpha.2](#checkpoint-v010-alpha2--conectividade-pública-validada)
- [Configuração privada — alpha.3](#configuração-local-e-privacidade-do-endpoint)
- [Publicação de fontes](#publicação-dos-fontes-no-github)
- [Como registrar alterações](#como-registrar-próximas-alterações)

## 2026-10-21 — Integração inicial

### Contexto observado

O checkout contém o jogo GBA e não contém o port SDL2 nem um Makefile_pc
preexistente. Não foi possível validar um executável PC/Android completo.

### Decisões

| Decisão | Justificativa | Consequência |
| --- | --- | --- |
| Incorporar ENet 1.3.18, commit fixo e licença | Dependência reproduzível sem instalação global | Fontes upstream preservadas em src/net/enet |
| Polling no CB2_Overworld com timeout zero | Manter mudanças do jogo na thread principal | Menus e batalhas suspendem o polling |
| Limitar a 64 eventos por atualização | Limitar trabalho de recepção em cada quadro | Backlog pode levar vários quadros |
| Canal 0 para posições; canal 1 para handshake e saída | Evitar retransmitir posições que já ficaram antigas | Ordenação entre canais exige cuidado no servidor |
| Serializar inteiros em big endian | Evitar dependência do endianness do cliente | Servidor deve converter os mesmos campos |
| Aceitar apenas IPv4 numérico | Evitar resolução DNS bloqueante | Nomes de servidor não são aceitos |
| Descartar sequência repetida ou antiga com aritmética serial | Suportar rollover de u32 | Saltos de metade do espaço de sequência são rejeitados |
| Reservar IDs locais 240–247 e verificar propriedade do objeto | Preservar NPCs quando o mapa reutiliza slots | Conflitos de IDs impedem criar aquele sprite |
| Remover objetos de rede sem alterar flags persistentes | API comum de remoção também grava flags do mapa | Função específica protegida por MULTIPLAYER |
| Excluir src/net do Makefile GBA | ENet depende de sockets do sistema operacional | Multiplayer exige build do port com MULTIPLAYER |
| Criar Makefile_pc para biblioteca estática | Port executável ausente neste checkout | Link final ainda depende do port |
| Usar -iquote include | include/strings.h do jogo conflita com strings.h da libc sob -Iinclude | Headers de sistema mantêm resolução correta |

### Validação automatizada

Comando: `make -f Makefile_pc network test-network`.
Resultado: biblioteca compilada no Linux e dois executáveis de testes aprovados.

| Arquivo | Comportamentos verificados |
| --- | --- |
| tests/net/client_test.c | Rejeição de endereço/porta inválidos; ausência de posições antes do welcome; hello confiável; canais; serialização big endian; intervalo de 50 ms; descarte de pacote truncado, vazio e direção inválida; eco do próprio jogador; saída confiável; desconexão; timeout; erro de polling; desligamento repetido |
| tests/net/sync_test.c | Tamanho do pacote; coordenadas com offset; sequência antiga e duplicada; rollover; saída e retorno ao mapa; preservação de NPC que reutiliza slot; pool de objetos cheio e recuperação; remoção de jogador |

O teste de cliente substitui a fronteira ENet por um simulador determinístico,
com relógio controlado e assert de timeout zero em cada polling. Não usa sockets.
O teste de sincronização simula as APIs de Object Events. Esses testes verificam
o código de cliente e sincronização; não validam a implementação interna do ENet,
transporte real, renderização, colisão ou execução no jogo.

### Pendências e limites conhecidos

- Falta teste com servidor ENet real e duas instâncias do port SDL2.
- Windows e Android não foram compilados neste ambiente.
- Inicialização e encerramento devem ser chamados pelo ciclo de vida do port.
- Chat, solicitações de batalha/troca e UI de conexão ainda não implementados.
- Sprites usam Brendan e reposicionamento por tile; animação/interpolação pendentes.
- Não há reconexão automática nem polling em menus/batalhas.
- Saída confiável e posição não confiável usam canais independentes: o servidor
  precisa impedir que estado atrasado recrie um jogador já desconectado.

## Como registrar próximas alterações

Para cada alteração, registrar o problema observado, a decisão e sua justificativa,
os arquivos afetados, o comando de teste, o resultado e o que ainda não foi validado.
Separar sempre resultado observado de comportamento apenas esperado.

## 2026-10-21 — Preparação da versão Linux

A ROM fornecida corresponde ao hash esperado. Como o checkout raiz é GBA,
foi obtido o port SDL2 upstream no commit
`db1cab3d2dc9f0e0a9f4a3d67983e5acf30a9a46`. A integração foi preparada em
`build/linux-port` para preservar as mudanças locais. Scripts versionados
permitem reproduzir a preparação e interrompem se os pontos de patch divergirem.
A ROM serve para validar a revisão; o port executa o código-fonte diretamente.

O port exige ponteiros de 32 bits, por isso mantém `-m32`. A sondagem do toolchain
falhou pela ausência de crt1.o/crti.o; SDL2 também não tinha metadados pkg-config.
A instalação autorizada das dependências parou na autenticação do sudo.
Não foi declarado sucesso de compilação nem execução do jogo.
Detalhes e testes manuais em [linux-playable.md](linux-playable.md).

### Correção das dependências Fedora 44

O comando inicial usava `SDL2-devel.i686`, que o DNF não resolveu. No Fedora 44,
o pacote de desenvolvimento é `sdl2-compat-devel.i686`; ele oferece a API SDL2
sobre SDL3. O comando em linux-playable.md foi corrigido. Não usar
`--skip-unavailable`: isso deixaria uma dependência necessária ausente.
Fonte: [pacote sdl2-compat-devel no Fedora](https://packages.fedoraproject.org/pkgs/sdl2-compat/sdl2-compat-devel/).

### Resultado do build e inicialização Linux

O usuário instalou as dependências i686. O pacote SDL2_image requer metadados
privados de codecs no pkg-config; para link dinâmico usamos os flags de SDL2 e
`-lSDL2_image` diretamente, evitando exigir pacotes de desenvolvimento dos codecs.
As bibliotecas de execução foram confirmadas por `ldd` sem itens ausentes.

O macro `abs` do jogo conflitou com stdlib.h: a inclusão de stdlib.h foi movida
antes de global.h no cliente e no hook do port. O port upstream já usa uma thread
SDL para AgbMain; os hooks de rede ficam nessa thread existente. No encerramento,
a thread de interface libera o semáforo de VBlank e aguarda a thread do jogo;
esta retorna de AgbMain e chama ShutdownNetworkClient. A interface não altera
os Object Events por meio do shutdown da rede.

O ELF i386 de 26 MB foi produzido com sucesso. O teste sem janela durou oito
segundos e registrou Task_Scene1_Load e frame 300 antes do timeout esperado 124.
Ainda não foi feita inspeção visual nem teste de input, áudio ou persistência.
O teste adicional `make -f Makefile_pc test-network-loopback` transmitiu mensagens
nos canais 0 e 1 por UDP real local; ambos passaram. Exige acesso a sockets locais.

### Avaliação da VM Oracle via SSH

Acesso autenticado por SSH com o usuário administrativo da VM de referência. Sistema Ubuntu 24.04.4 LTS
x86_64, duas CPUs, cerca de 954 MiB de RAM e 38 GiB livres em disco. Esses recursos
parecem adequados para testes iniciais de um relay pequeno; não houve benchmark.
Não foram encontrados gcc, make ou Docker. Há Python 3 e systemd com gerenciador
de usuário ativo. As regras INPUT terminam em REJECT, sem liberação UDP $POKE_NET_PORT.
A rede Oracle/NSG não foi consultada. Nenhum pacote, serviço ou regra foi alterado.
A implementação/deploy do servidor permanece pendente.

Um apelido em `~/.ssh/config` é usado como argumento de `ssh`; ele não cria
um comando de shell. Cada administrador define seu próprio destino SSH.

## 2026-10-21 — Servidor mínimo e primeiro checkpoint

Escopo acordado: base funcional antes de QoL ou mudanças de gameplay. Foi criado
[ROADMAP.md](../ROADMAP.md) com status e critérios de conclusão do primeiro marco.
A tag planejada para o checkpoint é `v0.1.0-alpha.1`, ainda com pendências
explícitas de rede pública e validação visual. Alterações foram feitas no branch
`multiplayer/initial-linux`; não há publicação/push automático.

### Decisões implementadas

- Servidor relay C/ENet separado do jogo, até oito conexões totais, porta UDP $POKE_NET_PORT.
- Nenhuma ROM ou save enviado ao servidor. Identidade atribuída pela sessão ENet.
- Protocolo aumentado para 2: JOIN confiável estabelece membros permitidos. LEAVE
  remove a permissão, evitando que posição atrasada recrie um jogador removido.
  Apenas envio de LEAVE pelo servidor não resolveria a independência dos canais.
- Snapshots periódicos recuperam posições descartadas por terem chegado antes do
  JOIN. Sequência repetida continua sendo rejeitada pela camada de sincronização.
- Validação de campos/canal/tamanho, timeout de handshake e limite por conexão.
- Serviço de usuário systemd com linger; limites de 128 MiB e uma CPU; restart
  automático em falha. Firewall tem serviço separado que libera só UDP $POKE_NET_PORT.
- Port isolado recebe os fontes de rede atualizados no build; launcher separado
  configura a VM. Saves e configurações permanecem locais.

### Evidências desta entrega

- `make -f Makefile_pc network server relay-test test-network`: aprovado.
- `python3 scripts/test_relay.py`: integração UDP local aprovada com cliente real.
- Mesmo teste com `relay-test32`, usando objetos ENet i386 da build: aprovado.
- Servidor compilado com GCC 13 no Ubuntu 24.04 da VM: aprovado.
- Integração executada em 127.0.0.1 dentro da VM: aprovada, inclusive após restart.
- Serviço ativo em 0.0.0.0:$POKE_NET_PORT; firewall específico ativo; boot completo não testado.
- Executável Linux recompilado com protocolo 2 e diagnósticos de conexão.
- Teste sem janela novamente aprovado: cena inicial e quadro 300 antes do timeout.
- Teste público para $POKE_NET_SERVER:$POKE_NET_PORT expirou. Contador UDP no firewall da VM
  ficou em zero: bloqueio anterior à VM, possivelmente Security List/NSG Oracle.
  A regra necessária foi solicitada ao usuário; não há acesso OCI configurado.

Os testes de cliente usam callbacks simulados para sprites; não demonstram dois
jogadores visíveis na tela. Não foram declarados concluídos testes manuais de
áudio, input, save, mudança de mapa ou gameplay prolongada.

### Correção detectada pelo teste final de encerramento

O port pode chamar VBlankIntrWait dentro dos próprios callbacks, antes de retornar
a AgbMain. A primeira tentativa de encerrar apenas após o loop externo não
cobria esse caminho: o teste final ficou aguardando a thread do jogo. Essa
implementação foi substituída antes do checkpoint.

O hook SDL agora mantém um sinalizador atômico de encerramento e um ponto de
saída setjmp na própria thread do jogo. VBlankIntrWait verifica o sinalizador
antes/depois da espera e retorna ao ponto de saída via longjmp na mesma thread.
Ali é executado ShutdownNetworkClient; a interface aguarda a thread terminar.
O teste tem kill-after de dois segundos para falhar em vez de esperar sem limite.
Isso substitui a descrição anterior de retorno via loop externo em AgbMain.

Resultado da correção de encerramento: teste sem janela aprovado; processo
finalizou após o timeout, log confirmou cena inicial e quadro 300. Não foi
necessário forçar encerramento na nova build. O processo travado da tentativa
anterior foi finalizado pelo PID específico do teste.

## Checkpoint v0.1.0-alpha.2 — Conectividade pública validada

Após o usuário adicionar a ingress rule UDP $POKE_NET_PORT na rede Oracle, os comandos
`build/pc-net/relay-test $POKE_NET_SERVER $POKE_NET_PORT` e
`build/pc-net/relay-test32 $POKE_NET_SERVER $POKE_NET_PORT` passaram. Cada execução usa o cliente
real do jogo e dois probes ENet. Foram verificados handshake, presença, posições,
pacotes antigos/inválidos, entrada tardia, saída e reconexão pela internet.

A alteração desta versão é de status, documentação e VERSION; não há mudança de
protocolo ou executável. A tag anterior permanece preservada. A comunicação
pública foi comprovada, mas os callbacks de sincronização no teste ainda são
simulados: não equivale a observar dois jogadores renderizados no jogo.

### Política de distribuição definida pelo usuário

O usuário determinou que o jogo compilado não pode ser publicado no GitHub;
apenas fontes e ferramentas de recompilação. Pacotes gerados são estritamente
locais. A política está em distribution.md e as exclusões de artefatos foram
ampliadas. O CI existente foi inspecionado: não há upload de ROM/jogo como
artefato ou release. Nenhum pacote/executável foi publicado nesta sessão.

## Configuração local e privacidade do endpoint

Por instrução do usuário, o endpoint da VM não pode constar no material publicável.
Foram removidos os valores da documentação, launchers, templates e pacotes futuros.
O jogador preenche um modelo vazio em arquivo ignorado pelo Git; o parser não
executa o conteúdo do arquivo e valida IPv4/porta. O ambiente pode sobrescrever
os campos. O jogo agora lê IP e porta do ambiente, sem porta padrão no hook.
O servidor exige uma porta explícita. O serviço já instalado na VM continua ativo,
com sua configuração privada anterior; templates públicos foram parametrizados.

Sete testes de configuração cobrem validação, precedência de ambiente, campos
obrigatórios, CRLF/normalização e ausência de execução de comandos. O teste de
integração com porta efêmera passou usando InitNetworkClientFromEnvironment.
A build Linux foi recompilada. Checkpoints anteriores também precisam de
sanitização, porque remover valores apenas no HEAD não protege o histórico.

## Publicação dos fontes no GitHub

Após autorização expressa do usuário, o branch de implementação e as três tags
sanitizadas foram publicados por SSH, de forma atômica. Auditoria prévia verificou
o histórico a enviar, metadados das tags e ausência de arquivos gerados do jogo ou
endpoint privado. O master não foi alterado. Builds, ROM e configuração pessoal
continuam fora do GitHub. As notas anteriores de ausência de push descrevem o
estado anterior a esta publicação.
