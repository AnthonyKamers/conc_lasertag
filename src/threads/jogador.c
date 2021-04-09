#include "config.h"
#include "arranjo.h"
#include "equipe.h"
#include "partida.h"
#include "prateleira.h"
#include "jogador.h"
#include "limpador.h"
#include "porteiro.h"

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/*============================================================================*
 * Protótipo das funções locais.                                              *
 *============================================================================*/

/**
 * @brief Funções que você deve implementar.
 *
 * A implementação deve ser realizada nas funções a baixo da função Main do
 * jogador.
 */
PRIVATE void jogador_escolhe_equipe(jogador_t * jogador);
PRIVATE void jogador_aloca_equipamento(jogador_t * jogador);
PRIVATE void jogador_espera_partida_comecar(jogador_t * jogador);
PRIVATE void jogador_joga_partida(jogador_t * jogador);
PRIVATE void jogador_espera_partida_terminar(jogador_t * jogador);
PRIVATE void jogador_libera_equipamento(jogador_t * jogador);
PRIVATE void jogador_sai_equipe(jogador_t * jogador);

/*============================================================================*
 * Responsabilidades do Jogador.                                              *
 *============================================================================*/

/**
 * @brief Main de um jogador.
 *
 * Descrição Geral:
 *
 * 1. Não modificar ou introduzir novas chamadas dentro do main do
 * jogador. Você deve implementar o que se espera de cada função seguindo
 * a descrição da mesma. Isto facilitará na avaliação do teu trabalho.
 *
 * 2. Você pode introduzir novas funções e estruturas para auxiliar, caso
 * necessário. Entretanto, essas chamadas abaixo são obrigatórias e devem
 * reproduzir o comportamento esperado.
 *
 * Seus objetivos são:
 * - Coordenar as threads (jogador, gerente e limpador) para que duas equipes
 *   de jogadores disputem uma partida de LazerGun.
 * - Jogadores vão ser criados aleatoriamente e tentarão participar de uma
 *   partida.
 * - O jogador deve iniciar tentando entrar em uma equipe (A ou B). Cada thread
 *   só pode estar em uma única equipe.
 * - Uma vez alocado em uma equipe, o jogador deve pegar um colete, um capacete
 *   e uma arma da prateleira (código responsável por gerenciar o alocação dos
 *   equipamentos).
 * - Jogadores prontos para a partida devem esperar que as duas equipes estejam
 *   completas.
 * - Se uma partida estiver em andamento, novos jogadores devem esperar a sua
 *   conclusão. Ou seja, eles não podem entrar em uma equipe enquanto uma
 *   partida estiver em andamento ou esteja cheia.
 * - Durante a partida, o jogador deve escolher aleatoriamente um jogador
 *   inimigo e causar dano a ele. Após isso, o jogador deve esperar um tempo
 *   aleatório para atacar novamente.
 * - O gerente deve esperar que todos os jogadores de uma equipe estejam mortos
 *   ou o tempo limite estourar para finalizar uma partida.
 * - O gerente também deve, periodicamente, curar todos os jogadores, caso
 *   eles não estejam mortos (vida > 0 e status != JOGADOR_MORTO).
 * - Jogadores mortos devem esperar a partida terminar.
 * - Depois de uma partida terminar, o jogador deve liberar a vaga da equipe
 *   e, depois, requisitar a limpeza dos equipamentos para o limpador.
 * - O limpador limpa os equipamentos e coloca eles novamente na prateleira.
 * - Quando todas as partidas terminarem, o programa deve finalizar a execução.
 *
 * ATENÇÃO: A criação e liberação dos recursos de um jogador é responsabilidade do
 * porteiro. O mesmo já está implementado e não pode ser alterado.
 *
 * ATENÇÃO: Você deve incrementar os contadores do sim_t referente ao jogadores
 * mortos.
 */
PUBLIC void * jogador_fn(void * arg)
{
	jogador_t * jogador;

	jogador = (jogador_t *) arg;

	plog("[jogador %d] Chegou na Arena.\n", jogador->id);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	jogador_escolhe_equipe(jogador);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	jogador_aloca_equipamento(jogador);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	jogador_espera_partida_comecar(jogador);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	jogador_joga_partida(jogador);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	jogador_espera_partida_terminar(jogador);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	jogador_sai_equipe(jogador);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	jogador_libera_equipamento(jogador);

	/* ATENÇÃO: Não remover a chamada a baixo. */
	porteiro_checklist(jogador);

	plog("[jogador %d] Vai embora.\n", jogador->id);

	return (NULL);
}

/*============================================================================*
 * jogador_escolhe_equipe()                                                   *
 *============================================================================*/

/**
 * @brief O jogador deve tentar entrar em uma equipe que ainda tenha vagas
 * disponíveis. Caso a partida já tenha iniciado, nenhum jogador pode entrar em
 * uma equipe.
 *
 * A quantidade de jogadores por equipe é constante e está definida na
 * variável global params.
 *
 * A variável global partida contém as estruturas das equipes. Você deve
 * coordenar os jogadores para entrarem em alguma dessas equipes.
 */
PRIVATE void jogador_escolhe_equipe(jogador_t * jogador)
{
	sem_wait(&partida->semaforo_deixa_escolher_equipe);  // semáforo para ver se pode começar a escolher a equipe

		jogador->status = JOGADOR_ESCOLHENDO_EQUIPE;

		plog("[jogador %d] Escolhendo a equipe.\n", jogador->id);

		/* Escolha uma equipe. */
		int equipe = aleatorio(0, 2);

		arranjo_t *jogadores = equipe == 0 ?
		(arranjo_t *) &partida->equipe_a.jogadores :
		(arranjo_t *) &partida->equipe_b.jogadores;

		// tenta acessar semáforo de uma equipe aleatória
		// se não conseguir, é porque está cheia,
		// então, acessa a outra equipe
		if (sem_trywait(&jogadores->semaforo) == 0) {

			plog("vou adicionar o jogador novo na equipe %d | tamanho equipe = %d \n", equipe, arranjo_tamanho(jogadores));
			arranjo_colocar(jogadores, jogador);
			jogador->equipe = equipe == 0 ? EQUIPE_A : EQUIPE_B;

			partida->jogadores_equipes++;
		} else {
			// se não conseguiu, tenta entrar na outra equipe
			equipe = equipe == 0 ? 1 : 0;

			jogadores = equipe == 0 ? 
			(arranjo_t *) &partida->equipe_a.jogadores :
			(arranjo_t *) &partida->equipe_b.jogadores;

			plog("vou adicionar o jogador novo na equipe %d | tamanho equipe = %d \n", equipe, arranjo_tamanho(jogadores));
			sem_wait(&jogadores->semaforo);
				arranjo_colocar(jogadores, jogador);
				jogador->equipe = equipe == 0 ? EQUIPE_A : EQUIPE_B;

				partida->jogadores_equipes++;
		}

		// plog("partida->jogadores_equipes = %d \n", partida->jogadores_equipes);
		// int semEquipe1, semEquipe2;
		// sem_getvalue(&partida->equipe_a.jogadores.semaforo, &semEquipe1);
		// sem_getvalue(&partida->equipe_b.jogadores.semaforo, &semEquipe2);

		// plog("semaforo equipe A = %d \n", semEquipe1);
		// plog("semaforo equipe B = %d \n", semEquipe2);


		// último jogador a entrar nas equipes vai settar semáforo do gerente para prosseguir
		if (partida->jogadores_equipes == 2 * params->jogadores_por_equipe) {
			plog("liberei o semáforo semaforo_gerente_espera_equipes! \n");
			sem_post(&partida->semaforo_gerente_espera_equipes);
		}

		plog("[jogador %d] Escolheu a equipe %d.\n", jogador->id, jogador->equipe);
}

/*============================================================================*
 * jogador_aloca_equipamento()                                                *
 *============================================================================*/

/**
 * @brief Após entrar numa equipe, o jogador vai até a prateleira pegar seus
 * equipamentos.
 *
 * Vá até o arquivo src/estruturas/prateleira.c para implementa-lá.
 */
PRIVATE void jogador_aloca_equipamento(jogador_t * jogador)
{
	jogador->status = JOGADOR_PEGANDO_EQUIPAMENTO;

	// tenta pegar um item da prateleira (se não conseguir, é porque não tem na prateleira)
	// só pega equipamento, se tiver disponível
	sem_wait(&partida->semaforo_equipamentos_disponiveis);
		prateleira_pega_equipamentos(&jogador->equipamentos);

		plog("[jogador %d] Alocou os equipamentos [%d, %d, %d].\n",
			jogador->id,
			jogador->equipamentos.colete,
			jogador->equipamentos.capacete,
			jogador->equipamentos.arma
		);
}

/*============================================================================*
 * jogador_espera_partida_comecar()                                           *
 *============================================================================*/

/**
 * @brief O jogador deve esperar que as equipes estejam completas
 * (params->jogadores_por_equipe em cada equipe) para começarem a jogar.
 */
PRIVATE void jogador_espera_partida_comecar(jogador_t * jogador)
{
	jogador->status = JOGADOR_ESPERANDO;

	plog("[jogador %d] Esperando partida começar.\n", jogador->id);

	sem_wait(&partida->semaforo_wait_partida);
		// incrementar contador de jogadores esperando
		partida->jogadores_esperando++;

		plog("partida->jogadores_esperando = %d \n", partida->jogadores_esperando);

		// último jogador a esperar, vai settar semáforo do gerente para avançar e iniciar o jogo
		if (partida->jogadores_esperando == 2 * params->jogadores_por_equipe) {
			sem_post(&partida->semaforo_gerente_jogadores_esperando);
		}

		// espera gerente liberar para conseguir jogar
		sem_wait(&partida->semaforo_comecar_partida);

}

/*============================================================================*
 * jogador_joga_partida()                                                     *
 *============================================================================*/

/**
 * @brief Enquanto a partida estiver em andamento, jogadores vivos devem
 * procurar um alvo aleatório na equipe adversária e causar
 * um dano aleatório (params->dano_min <= dano < params->dano_max) no alvo.
 * Caso a vida do adversário fique menor ou igual a zero, o adversário deve ser
 * movido pro estado MORTO e não pode mais continuar jogando.
 * Depois de atacar, o jogador deve esperar um tempo aleatório antes de atacar
 * novamente (params->delay_min <= t < params->delay_max);
 */
PRIVATE void jogador_joga_partida(jogador_t * jogador)
{
	jogador->status = JOGADOR_JOGANDO;

	plog("[jogador %d] Jogando.\n", jogador->id);

	// enquanto partida estiver acontecendo (status partida == PARTIDA_INICIADA)
	// e status do jogador é NÃO MORTO:
	// faz jogador jogar (encontra inimigos e tira vida aleatória)
	while (
		partida->status != PARTIDA_FINALIZADA &&
		jogador->status != JOGADOR_MORREU
	) {
		int dano_now = aleatorio(params->dano_min, params->dano_max);

		arranjo_t *equipe_adversaria = jogador->equipe == EQUIPE_A ?
			(arranjo_t*) &partida->equipe_b.jogadores :
			(arranjo_t*) &partida->equipe_a.jogadores;

		arranjo_t *jogadores_adversarios = filtrar_jogadores(equipe_adversaria, JOGADOR_JOGANDO);
		int tamanhoJogadores = arranjo_tamanho(jogadores_adversarios);

		if (tamanhoJogadores != 0) {
			int inimigo_ataque_index = aleatorio(0, tamanhoJogadores);
			jogador_t *inimigo_ataque = (jogador_t*) jogadores_adversarios->conteudo[inimigo_ataque_index];

			inimigo_ataque->vida -= dano_now;

			// plog("jogador %d ainda está jogando | status partida = %d | status jogador = %d \n",
			// 	jogador->id,
			// 	partida->status,
			// 	jogador->status
			// );

			plog("------------------->> jogador %d tirou %d de vida do jogador %d q está com vida %d \n", 
				jogador->id,
				dano_now,
				inimigo_ataque->id,
				inimigo_ataque->vida
			);

			// se jogador morreu, setta para JOGADOR_MORREU
			// adiciona em simulador a quantidade de mortos
			if (inimigo_ataque->vida <= 0) {
				inimigo_ataque->status = JOGADOR_MORREU;
				sim->jogadores_mortos += 1;
			}
		}

		// arranjo_destruir(jogadores_adversarios);
		// free(jogadores_adversarios);

		// esperar tempo aleatório antes de atacar novamente
		msleep(aleatorio(params->delay_min, params->delay_max));
	}

	plog("jogador saiu do while -> status jogador = %d | status partida = %d \n",
		jogador->status,
		partida->status
	);

	plog("[jogador %d] Saindo do jogo.\n", jogador->id);
}

/*============================================================================*
 * jogador_espera_partida_terminar()                                          *
 *============================================================================*/

/**
 * @brief Jogadores que morreram ou terminaram de jogar devem esperar a partida
 * terminar nesta função.
 */
PRIVATE void jogador_espera_partida_terminar(jogador_t * jogador)
{
	plog("[jogador %d] Esperando a partida terminar.\n", jogador->id);

	sem_wait(&partida->semaforo_saindo_partida);
}

/*============================================================================*
 * jogador_sai_equipe()                                                       *
 *============================================================================*/

/**
 * @brief Após o fim da partida, o jogador deve liberar a vaga da equipe para
 * outra pessoa.
 *
 * ATENÇÃO: Não altere a ordem das operações no main do jogador, ou seja,
 * o jogador deve sair da equipe antes de liberar os equipamentos.
 */
PRIVATE void jogador_sai_equipe(jogador_t * jogador)
{
	plog("[jogador %d] Libera vaga.\n", jogador->id);

	/* Sair da equipe. */
	arranjo_t *arranjoJogadoresJogador = jogador->equipe == EQUIPE_A ? 
		(arranjo_t *) &partida->equipe_a.jogadores :
		(arranjo_t *) &partida->equipe_b.jogadores;

	arranjo_remover(arranjoJogadoresJogador, (void *) jogador);  // remove da equipe
	sem_post(&arranjoJogadoresJogador->semaforo);  // abre espaço para conseguir entrar na equipe

	partida->jogadores_ja_sairam++;

	if (partida->jogadores_ja_sairam >= 2 * params->jogadores_por_equipe) {
		// libera semáforo para gerente poder continuar e
		// criar nova partida
		plog("último jogador a sair vai liberar para gerente começar nova partida \n");
		sem_post(&partida->semaforo_gerente_comeca_partida);
	}
}

/*============================================================================*
 * jogador_libera_equipamento()                                               *
 *============================================================================*/

/**
 * @brief Após sair da equipe, o jogador deve solicitar ao limpador que
 * higienize os equipamento. O limpador deve ser responsável por devolvê-los
 * à prateleira.
 */
PRIVATE void jogador_libera_equipamento(jogador_t * jogador)
{
	jogador->status = JOGADOR_LIBERANDO_EQUIPAMENTO;

	plog("[jogador %d] Libera equipamentos.\n", jogador->id);

	/* Libera equipamentos. */

	// travar mutex do limpador
	// somente um jogador pode requisitar limpeza por vez
	pthread_mutex_lock(&partida->mutex_limpador);
		limpador_requisita_limpeza(&jogador->equipamentos);
	pthread_mutex_unlock(&partida->mutex_limpador);
}

