#include "sppsAGHInitialisation.h"
#include <iostream>
inline bool ContainsRP(t_Recepteur_P* recepteurTest, std::vector<t_Recepteur_P*>* rpList)
{
	for(std::size_t idRP=0;idRP<rpList->size();idRP++)
	{
		if(rpList->at(idRP)==recepteurTest)
			return true;
	}
	return false;
}

//Effectue une translation de distTranslation des sources positionn�es sur le sommet d'un t�dra�dre
void TranslateSourceAtTetrahedronVertex(std::vector<t_Source*>& lstSource,t_TetraMesh* tetraMesh)
{
	float distTranslation(0.005f);
	for(std::vector<t_Source*>::iterator itsrc=lstSource.begin();itsrc!=lstSource.end();itsrc++)
	{
		for(unsigned short idvertex=0;idvertex<4;idvertex++)
		{
			if((*itsrc)->Position.distance(tetraMesh->nodes[(*itsrc)->currentVolume->sommets[idvertex]])<BARELY_EPSILON)
			{
				//Translation de distTranslation vers le centre du t�tra�dre
				vec3 centerTetra=GetGTetra(tetraMesh->nodes[(*itsrc)->currentVolume->sommets.a],
					tetraMesh->nodes[(*itsrc)->currentVolume->sommets.b],
					tetraMesh->nodes[(*itsrc)->currentVolume->sommets.c],
					tetraMesh->nodes[(*itsrc)->currentVolume->sommets.d]);
				std::cout<<"Source at tetrahedron vertex, move source position from ["<<(*itsrc)->Position.x<<";"<<(*itsrc)->Position.y<<";"<<(*itsrc)->Position.z<<"]";
				(*itsrc)->Position=(*itsrc)->Position+((centerTetra-(*itsrc)->Position)*distTranslation);
				std::cout<<" to ["<<(*itsrc)->Position.x<<";"<<(*itsrc)->Position.y<<";"<<(*itsrc)->Position.z<<"]"<<std::endl;
				break;
			}
		}
	}
}


/**
 * M�thode r�cursive de propagation de liaison entre un r�cepteur ponctuel et les t�tra�dres voisins.
 * Pas d'appel r�cursif si le t�tra�dre courant est d�j� li� au r�cepteur ponctuel ou si le r�cepteur ponctuel ne s'�tend pas jusqu'a se volume
 * Si une collision est d�tect� alors chaque tetra�dre voisin est test�
 * @see ExpandRecepteurPTetraLocalisation
 **/
void RecursiveTetraTest( t_Recepteur_P* recepteurTest, const decimal& rayon, t_Tetra* currentTetra , t_TetraMesh* tetraMesh)
{
	if(!currentTetra)
		return;
	if(currentTetra->linkedRecepteurP)
	{
		if(ContainsRP(recepteurTest,currentTetra->linkedRecepteurP))
		{
			return; //R�cepteur ponctuel deja associ� au sous volume
		}
	}
	for(int idfaceTri=0;idfaceTri<4;idfaceTri++)
	{
		decimal minLength=sqrtf(ClosestDistanceBetweenDotAndTriangle(
			tetraMesh->nodes[currentTetra->faces[idfaceTri].indiceSommets.a],
			tetraMesh->nodes[currentTetra->faces[idfaceTri].indiceSommets.b],
			tetraMesh->nodes[currentTetra->faces[idfaceTri].indiceSommets.c],
			recepteurTest->position,NULL,NULL));
		if(minLength<=rayon) //Si la face est travers� par la sphere
		{
			if(!currentTetra->linkedRecepteurP)
				currentTetra->makeTabRecp();
			currentTetra->linkedRecepteurP->push_back(recepteurTest);
			for(int idVoisin=0;idVoisin<4;idVoisin++)
			{
				RecursiveTetraTest(recepteurTest, rayon, currentTetra->voisins[idVoisin], tetraMesh);
			}
			return;
		}
	}
}

void ExpandRecepteurPTetraLocalisation(t_TetraMesh* tetraMesh,std::vector<t_Recepteur_P*>* lstRecepteurP,Core_Configuration& configManager)
{
	decimal rayonRecepteurP=*configManager.FastGetConfigValue(Core_Configuration::FPROP_RAYON_RECEPTEURP);
	decimal volumeRP=(pow(*configManager.FastGetConfigValue(Core_Configuration::FPROP_RAYON_RECEPTEURP),3)*M_PI*4.)/3.;
	for(std::size_t idrecp=0;idrecp<lstRecepteurP->size();idrecp++)
	{
		if(lstRecepteurP->at(idrecp)->indexTetra<tetraMesh->tetraedresSize)
		{
			t_Tetra* recpTetra=&tetraMesh->tetraedres[lstRecepteurP->at(idrecp)->indexTetra];
			//Calcul de la nouvelle valeur de normalisation par rapport au volume cdtVol qui n'est plus celui du t�tra�dre

			//lstRecepteurP->at(idrecp)->cdt_vol=pow((*configManager.FastGetConfigValue(Base_Core_Configuration::FPROP_CELERITE)),2.f)*(*configManager.FastGetConfigValue(Base_Core_Configuration::FPROP_TIME_STEP))*(*configManager.FastGetConfigValue(Base_Core_Configuration::FPROP_RHO))/(volumeRP);
			lstRecepteurP->at(idrecp)->cdt_vol=((*configManager.FastGetConfigValue(Base_Core_Configuration::FPROP_CELERITE))*(*configManager.FastGetConfigValue(Base_Core_Configuration::FPROP_RHO)))/volumeRP;
			for(int idVoisin=0;idVoisin<4;idVoisin++)
			{
				RecursiveTetraTest(lstRecepteurP->at(idrecp), rayonRecepteurP, recpTetra->voisins[idVoisin], tetraMesh);
			}
		}
	}
}
