#include "config.h"
#include "partida.h"
#include "jogador.h"

/**
 * ATENÇÃO: Você pode adicionar novas funções com PUBLIC para serem usadas por
 * outros arquivos e adicionar a assinatura da função no .h referente.
 */

/*============================================================================*
 * Protótipo das funções locais.                                              *
 *============================================================================*/

PRIVATE void gerente_coordena_partida(void);

/*============================================================================*
 * gerente_setup()                                                            *
 *============================================================================*/

/**
 * @brief Configura os recursos locais do gerente se houverem.
 */
PUBLIC void gerente_setup(void)
{
}

/*============================================================================*
 * gerente_cleanup()                                                          *
 *============================================================================*/

/**
 * @brief Limpa os recursos locais do gerente se houverem.
 */
PUBLIC void gerente_cleanup(void)
{
}

/*============================================================================*
 * Responsabilidades do Gerente.                                              *
 *============================================================================*/

/**
 * @brief Main do gerente.
 *
 * Descrição: O Gerente deve coordenar as partidas, ou seja:
 * - Esperar que as equipes se formarem.
 * - Iniciar a partida.
 * - Enquanto o tempo de partida não ultrapassar params->partida_tempo_max
 *   e ainda existem jogadores vivos em ambas as equipes:
 * --- Esperar params->delay_gerente
 * --- Perioricamente, curar todos os jogadores vivos com params->dano_cura.
 *
 * ATENÇÃO: Você deve incrementar os contadores do sim_t referente a partida
 * e jogadores curados.
 */
PUBLIC void * gerente_fn(void * arg)
{
	plog("[gerente] Iniciou.\n");

	// enquanto o tempo da partida não ultrapassar o tempo máximo (para cada partida)
	while (tempo_partida <= params->partida_tempo_max) {
		// tick
		msleep(params->delay_gerente);
		tempo_partida += params->delay_gerente;

		equipe_t equipeA = partida->equipe_a;
		equipe_t equipeB = partida->equipe_b;

		// se as equipes estiverem prontas -> inicia a partida
		if (
			(arranjo_tamanho(&equipeA.jogadores) == equipeA.capacidade) &&
			(arranjo_tamanho(&equipeB.jogadores) == equipeB.capacidade) &&
			partida->status == PARTIDA_NAO_PREPARADA
		) {
			partida->status = PARTIDA_PREPARADA;
		}

		// settar de PREPARADA para INICIADA [se todos jogadores estiverem esperando]
		// printf("are_todos_esperando() from gerente = %d \n", are_todos_esperando());
		if (
			are_todos_esperando() &&
			partida->status == PARTIDA_PREPARADA
		) {
			plog("gerente vai iniciar partida \n");
			partida->status = PARTIDA_INICIADA;
		}

		// se uma equipe está toda morta ou se o tempo acabou
		//	-> setta partida para FINALIZADA
		//	-> chama partida_nomeia_vencedores()
		if (
			partida->status == PARTIDA_INICIADA &&
			(
				(quantidade_vivos(equipeA) == 0 && arranjo_tamanho(&equipeA.jogadores) > 0) ||
				(quantidade_vivos(equipeB) == 0 && arranjo_tamanho(&equipeB.jogadores) > 0) ||
				tempo_partida >= params->partida_tempo_max
			)
		) {
			partida->status = PARTIDA_FINALIZADA;
			partida_nomeia_vencedores(tempo_partida);
		}

		// chama coordena_partida (para curar jogadores)
		gerente_coordena_partida();
	}

	return (NULL);
}

/*============================================================================*
 * gerente_cura_jogadores()                                                   *
 *============================================================================*/

/**
 * @brief Gerente cura os jogadores.
 */
PRIVATE int gerente_cura_jogadores(void)
{
	int curados = 0;
	int qtdGeral = params->jogadores_por_equipe;

	plog("[gerente] Curando jogadores.\n");

	arranjo_t *jogadoresA = &partida->equipe_a.jogadores;
	arranjo_t *jogadoresB = &partida->equipe_b.jogadores;

	for (int i = 0; i < qtdGeral; i++) {
		jogador_t *jogadorNowA = (jogador_t *) jogadoresA->conteudo[i];
		jogador_t *jogadorNowB = (jogador_t *) jogadoresB->conteudo[i];

		// curar jogadorA[i] e jogadorB[i]
		jogadorNowA->vida += params->dano_cura;
		sim->jogadores_curados++;

		jogadorNowB->vida += params->dano_cura;
		sim->jogadores_curados++;
	}

	return (curados);
}

/*============================================================================*
 * gerente_coordena_partida()                                                 *
 *============================================================================*/

/**
 * @brief Loop principal da partida.
 */
PRIVATE void gerente_coordena_partida(void)
{
	// plog("[gerente] Coordenando partida.\n");

	equipe_t equipeA = partida->equipe_a;
	equipe_t equipeB = partida->equipe_b;

	// enquanto houver tempo de partida e partida estiver rodando
	//	-> cura periodicamente os jogadores
	while (
		tempo_partida < params->partida_tempo_max &&
		partida->status == PARTIDA_INICIADA &&
		quantidade_vivos(equipeA) > 0 &&
		quantidade_vivos(equipeB) > 0
	) {
		// espera delay_gerente e cura jogadores após isso
		msleep(params->delay_gerente);
		gerente_cura_jogadores();
	}

	// if (false)
	// 	gerente_cura_jogadores();
}

