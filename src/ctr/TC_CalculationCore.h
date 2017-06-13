#include "input_output/reportmanager.h"
#include "data_manager/core_configuration.h"
#include <input_output/progressionInfo.h>
#include <tools/octree44.hpp>

#ifndef __CALC_CORE__
#define __CALC_CORE__


/**
 * @file CalculationCore.h
 * @brief Moteur de calcul bas� sur le calcul par th�orie classique
 */

/**
 * @brief Moteur de calcul bas� sur le calcul par th�orie classique
 */
class CalculationCore
{
public:
	/**
	 * @brief Param�tres du calcul de propagation acoustique
	 *
	 * Structure de donn�es donnant les informations sur les param�tres globaux de calcul
	 */
	struct CONF_CALCULATION
	{
		ReportManager::t_ParamReport reportParameter;
	};
	t_mainData mainData;
	std::vector<t_TC_RecepteurPonctuel> recepteurPList;
	/**
	 * @brief Constructeur du moteur de calcul.
	 * 
	 * @param _pVertices Tableau de points du mod�le
	 * @param _pGroups Tableau de faces du mod�le
	 * @param intersTool Outils de calcul d'intersection
	 * @param confEnv Configuration g�n�rale de l'environnement
	 * @param _reportTool Classe d'enregistrements des r�sultats de calculs.
	 * @param _configurationTool Classe de gestion de configuration
	 */
	CalculationCore(t_Mesh& _sceneMesh,t_TetraMesh& _sceneTetraMesh,CONF_CALCULATION &confEnv, Core_Configuration &_configurationTool);
	~CalculationCore();
	/**
	 * Execute le calcul pour une particule
	 * @param configurationP Configuration de la particule
	 * @return Vrai si le calcul c'est effectu� avec succ�s 
	 */
	bool Run();

private:
octreeTool::Octree44* modelOctree;
octreeTool::spaceElementContainer lstFaces;
t_Mesh *sceneMesh;
t_TetraMesh *sceneTetraMesh;
Core_Configuration *configurationTool;
CONF_CALCULATION confEnv;
progressionInfo progressOutput;
decimal Get_A_Sabine ( entier idFreq );
decimal Get_A_Eyring ( entier idFreq );

decimal Get_TR_( decimal A, entier idFreq );
decimal Get_L_lin( decimal A, entier idFreq );
decimal Get_L( decimal L_lin );
decimal Get_Champ_Direct_Lin( entier idFreq, vec3 positionRecepteur, bool angleAttenuation=false,vec3 dotNormal=vec3() );
decimal Get_Recepteur_P_Direct( decimal  Champ_Direct_Lin );
decimal Get_Recepteur_P_Champ_Total ( decimal  Champ_Direct_Lin, decimal L_lin );
decimal Get_Recepteur_S_Direct( decimal  Champ_Direct_Lin );
decimal Get_Recepteur_S_Champ_Total ( decimal  Champ_Direct_Lin, decimal L_lin );

decimal Get_Volume_Scene( );
/*
 * Calcul des valeurs globales par bandes
 */
void RunStep1(MODE_CALCUL modeCalcul);
/*
 * Calcul des valeurs par r�cepteurs ponctuels
 */
void RunStep2(progressOperation* parentProgressOp);
/*
 * Calcul des valeurs par r�cepteurs de surface
 */
void RunStep3( progressOperation* parentProgressOp );
/**
 * Calcul des valeurs pour les r�cepteurs surfacique en mode Champ Direct
 * @param idFreq Bande de fr�quence � calculer
 * @param[out] recepteurS Liste des recepteurs surfacique
 */
void SetEnergyDirect( entier idFreq, std::vector<r_Surf*>& recepteurS );
/**
 * Calcul des valeurs pour les r�cepteurs surfacique de coupe en mode Champ Direct
 * @param idFreq Bande de fr�quence � calculer
 * @param[out] recepteurS Liste des recepteurs surfacique
 */
void SetEnergyDirectCut( entier idFreq, std::vector<r_SurfCut*>& recepteurS );
/**
 * Calcul des valeurs pour les r�cepteurs surfaciques dans le mode de calcul pass� en param�tre
 * @param idFreq Bande de fr�quence � calculer
 * @param modeCalcul Th�orie de calcul
 * @param[in] recepteurS Liste des recepteurs surfacique calcul� en lin�aire
 * @param[out] recepteurS Liste des recepteurs surfacique
 */
void SetEnergyLin( MODE_CALCUL modeCalcul, entier idFreq , std::vector<r_Surf*>& recepteurSDirect, std::vector<r_Surf*>& recepteurSLin );
/**
 * Calcul des valeurs pour les r�cepteurs surfaciques de coupe dans le mode de calcul pass� en param�tre
 * @param idFreq Bande de fr�quence � calculer
 * @param modeCalcul Th�orie de calcul
 * @param[in] recepteurS Liste des recepteurs surfacique calcul� en lin�aire
 * @param[out] recepteurS Liste des recepteurs surfacique
 */
void SetEnergyLinCut( MODE_CALCUL modeCalcul, entier idFreq , std::vector<r_SurfCut*>& recepteurSDirect, std::vector<r_SurfCut*>& recepteurSLin );

/**
 * Retourne vrai si la ligne entre les deux points est travers� par une face de la sc�ne
 */
bool IsCollisionWithScene(vec3 from,vec3 to);


};

#endif