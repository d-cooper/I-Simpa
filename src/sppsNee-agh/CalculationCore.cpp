#include "CalculationCore.h"
#include "tools/collision.h"
#include "tools/dotreflection.h"
#include "tools/brdfreflection.h"
#include "tools/dotdistribution.h"
#include <iostream>
#ifndef INTSIGN
	#define INTSIGN(x) ((x < 0) ? -1 : 1 );
#endif
#define TetraFaceTest(idFace) sommetsFace=configurationP.currentTetra->faces[idFace].indiceSommets;\
			if(configurationP.currentTetra->faces[idFace].normal.dot(translationVector)<EPSILON)\
				if(collision_manager::intersect_triangle(posPart,dirPart,scenenodes[sommetsFace.a].v,scenenodes[sommetsFace.b].v,scenenodes[sommetsFace.c].v,&t,&u,&v)==1)\
					return idFace;

void printVec(vec3 inf)
{
	using namespace std;
	std::string retStr;
	cout<<"["<<inf.x<<";"<<inf.y<<";"<<inf.z<<"]";
}



CalculationCore::CalculationCore(t_Mesh& _sceneMesh,t_TetraMesh& _sceneTetraMesh,CONF_CALCULATION &_confEnv, Core_Configuration &_configurationTool,ReportManager* _reportTool)
:confEnv(_confEnv)
{
	sceneMesh=&_sceneMesh;
	configurationTool=&_configurationTool;
	sceneTetraMesh=&_sceneTetraMesh;
	reportTool=_reportTool;
	doDirectSoundCalculation = false;
}

void CalculationCore::SetNextParticleCollisionWithObstructionElement(CONF_PARTICULE &configurationP)
{
	if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_ENCOMBREMENT) && configurationP.currentTetra->volumeEncombrement)
	{
		//Tirage al�atoire de la distance
		configurationP.distanceToNextEncombrementEle=-configurationP.currentTetra->volumeEncombrement->encSpectrumProperty[configurationP.frequenceIndex].lambda*log((float)(1-GetRandValue()));
	}
}

void CalculationCore::CalculateDirectSound(CONF_PARTICULE prototypeParticle, t_Source& sourceInfo, float distancePerTimeStep)
{
	float receiverRadius = *configurationTool->FastGetConfigValue(Core_Configuration::FPROP_RAYON_RECEPTEURP);

	CONF_PARTICULE shadowRay = prototypeParticle;
	shadowRay.position = sourceInfo.Position;
	shadowRay.isShadowRay = true;
	shadowRay.outputToParticleFile = false;

	for each (t_Recepteur_P* receiver in configurationTool->recepteur_p_List)
	{
		vec3 toReceiver = (receiver->position - shadowRay.position);
		shadowRay.targetReceiver = receiver;
		shadowRay.direction = toReceiver;
		shadowRay.direction.normalize();
		shadowRay.direction *= distancePerTimeStep;

		if (VisabilityTest(shadowRay, receiver->position)) {
			double solidAngle = (M_PI*receiverRadius*receiverRadius) / (toReceiver.length()*toReceiver.length());

			//0.66 is normalization factor used to acount for not treating receiver as sphere 
			//- points far from sphere center ar treated with the same weight as ones in the middle,
			//proper weight schould be proportional to length of intersection, but it is hard to be evaluated quickly
			shadowRay.energie = (1 / (4 * M_PI))*solidAngle*0.66*sourceInfo.bandeFreqSource[shadowRay.frequenceIndex].w_j;
			confEnv.duplicatedParticles.push_back(shadowRay);
		}else
		{
			std::cout << "Receiver " << receiver->lblRp << " is not visible from source " << sourceInfo.sourceName << std::endl;
		}
	}
}

bool CalculationCore::Run(CONF_PARTICULE configurationP)
{
	decimal densite_proba_absorption_atmospherique=configurationTool->freqList[configurationP.frequenceIndex]->densite_proba_absorption_atmospherique;
	SetNextParticleCollision(configurationP);						//1er test de collision
	SetNextParticleCollisionWithObstructionElement(configurationP);	// Test de collision avec objet virtuel encombrant
	//Au premier pas de temps il faut enregistrer l'energie de la particule dans la maille courante
	//configurationP.stateParticule=PARTICULE_STATE_ALIVE;
	reportTool->ParticuleGoToNextTetrahedra(configurationP,configurationP.currentTetra);
	while(configurationP.stateParticule==PARTICULE_STATE_ALIVE && configurationP.pasCourant<confEnv.nbPasTemps)
	{
		//Test d'absorption atmosph�rique
		if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_ABS_ATMO))
		{
			//Test de m�thode de calcul
			if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_ENERGY_CALCULATION_METHOD))
			{ //Energetique
				configurationP.energie*=densite_proba_absorption_atmospherique;
				if(configurationP.energie<=configurationP.energie_epsilon)
				{
					configurationP.stateParticule=PARTICULE_STATE_ABS_ATMO;
				}
			}else{
				//Al�atoire
				if(GetRandValue()>=densite_proba_absorption_atmospherique)
				{
					configurationP.energie=0;	// La particule est d�truite
					configurationP.stateParticule=PARTICULE_STATE_ABS_ATMO;
				}
			}
		}

		if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
			Movement(configurationP);	//Effectue un mouvement sur la distance restante


		//Fin du pas de temps, la particule a effectu� aucune ou plusieurs collisions
		if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
		{
			reportTool->RecordTimeStep(configurationP);
			if(configurationP.currentTetra->z!=-1)								//Si le milieu n'est pas homog�ne
			{
				OnChangeCelerite(configurationP,configurationP.currentTetra);	//On calcul le changement de direction d� au gradiant de c�l�rit�
				SetNextParticleCollision(configurationP);
			}
			configurationP.pasCourant++;
		}
	}
	switch(configurationP.stateParticule)
	{
		case PARTICULE_STATE_ALIVE:
			reportTool->statReport.partAlive++;
			break;
		case PARTICULE_STATE_ABS_SURF:
			reportTool->statReport.partAbsSurf++;
			break;
		case PARTICULE_STATE_ABS_ATMO:
			reportTool->statReport.partAbsAtmo++;
			break;
		case PARTICULE_STATE_ABS_ENCOMBREMENT:
			reportTool->statReport.partAbsEncombrement++;
			break;
		case PARTICULE_STATE_LOOP:
			reportTool->statReport.partLoop++;
			break;
		case PARTICULE_STATE_LOST:
			reportTool->statReport.partLost++;
			break;
		case PARTICULE_STATE_SHADOW_RAY_REACHED_DST:
			reportTool->statReport.partShadowRay++;
			break;

	}
	reportTool->statReport.partTotal++;
	return true;
}

void CalculationCore::Movement(CONF_PARTICULE &configurationP)
{
	decimal deltaT=*configurationTool->FastGetConfigValue(Core_Configuration::FPROP_TIME_STEP) ;
	decimal distanceSurLePas=configurationP.direction.length();
	decimal celeriteLocal=distanceSurLePas/deltaT;
	decimal faceDirection;
	bool collisionResolution=true; //On test de nouveau la collision dans le pas de temps courant si cette valeur est � vrai
	int iteration=0;
	decimal distanceCollision=0.f;
	decimal distanceToTravel=0.f;
	while(collisionResolution && configurationP.stateParticule==PARTICULE_STATE_ALIVE)
	{
		iteration++;
		collisionResolution=false;
		//Si il y a collision avec une face (avec prise en compte de la distance parcourue)
		distanceCollision=(configurationP.nextModelIntersection.collisionPosition-configurationP.position).length();
		distanceToTravel=celeriteLocal*(deltaT-configurationP.elapsedTime);

		//Test de collision avec un �l�ment de l'encombrement entre la position de la particule et une face du tetrah�dre courant.
		if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_ENCOMBREMENT) && distanceToTravel>=configurationP.distanceToNextEncombrementEle && distanceCollision>configurationP.distanceToNextEncombrementEle && configurationP.currentTetra->volumeEncombrement)
		{
			//Collision avec un �l�ment virtuel de l'encombrement courant

			//Test d'absorption

			if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_ENERGY_CALCULATION_METHOD))
			{
				//Energ�tique
				configurationP.energie*=(1-configurationP.currentTetra->volumeEncombrement->encSpectrumProperty[configurationP.frequenceIndex].alpha);
				if(configurationP.energie<=configurationP.energie_epsilon)
				{
					configurationP.stateParticule=PARTICULE_STATE_ABS_ENCOMBREMENT;
					return;
				}
			}else{
				//Al�atoire
				if( GetRandValue()<=configurationP.currentTetra->volumeEncombrement->encSpectrumProperty[configurationP.frequenceIndex].alpha)
				{
					//Absorb�
					configurationP.energie=0.f;
					configurationP.stateParticule=PARTICULE_STATE_ABS_ENCOMBREMENT;
					return;
				}
			}
			//N'est pas absorb�

			//On incr�mente le temps de parcourt entre la position avant et apr�s collision avec l'encombrement virtuel
			configurationP.elapsedTime+=configurationP.distanceToNextEncombrementEle/celeriteLocal;
			//On place la particule sur la position de collision
			FreeParticleTranslation(configurationP,(configurationP.direction/configurationP.direction.length())*configurationP.distanceToNextEncombrementEle);
			collisionResolution=true;
			//On change la direction de la particule en fonction de la loi de distribution
			vec3 newDir;
			switch(configurationP.currentTetra->volumeEncombrement->encSpectrumProperty[configurationP.frequenceIndex].law_diffusion)
			{
				case DIFFUSION_LAW_UNIFORM:
					ParticleDistribution::GenSphereDistribution(configurationP,configurationP.direction.length());
					break;
				case DIFFUSION_LAW_REFLEXION_UNIFORM:
					newDir=ReflectionLaws::FittingUniformReflection(configurationP.direction);
					newDir.normalize();
					configurationP.direction=newDir*configurationP.direction.length();
					break;
				case DIFFUSION_LAW_REFLEXION_LAMBERT:
					newDir=ReflectionLaws::FittingLambertReflection(configurationP.direction);
					newDir.normalize();
					configurationP.direction=newDir*configurationP.direction.length();
					break;
				
			};
			//Calcul du nouveau point de collision
			SetNextParticleCollision(configurationP);
			SetNextParticleCollisionWithObstructionElement(configurationP);
		}else if(distanceCollision<=distanceToTravel) // && configurationP.nextModelIntersection.idface!=-1
		{
			//Enregistrement de l'�nergie pass� � la paroi
			reportTool->ParticuleCollideWithSceneMesh(configurationP);
			
			vec3 vecTranslation=configurationP.nextModelIntersection.collisionPosition-configurationP.position;
			//On incr�mente le temps de parcourt entre la position avant et apr�s collision
			configurationP.elapsedTime+=(vecTranslation/configurationP.direction.length()).length()*deltaT;

			//On place la particule sur la position de collision
			FreeParticleTranslation(configurationP,vecTranslation);

			// R�cuperation de l'information de la face
			t_cFace* faceInfo=NULL;

			#ifdef UTILISER_MAILLAGE_OPTIMISATION
			faceInfo=configurationP.currentTetra->faces[configurationP.nextModelIntersection.idface].face_scene;

			//test de passage d'un t�tra�dre � un autre

			//Vrai si la paroi est anormalement orient�e
			bool doInvertNormal(false);
			if(faceInfo)
			{
				faceDirection=configurationP.direction.dot(faceInfo->normal);
				doInvertNormal=(faceDirection<=-BARELY_EPSILON);
			}
			//On traverse la paroi du tetrahedre si (pas de r�solution de collision si)
			//	- Ce n'est pas une surface du mod�le
			//  - (ou) Elle n'est pas orient�e vers nous et le mat�riau n'affecte les surfaces sur une orientation
			//  - (ou) Cette surface est un encombrement et qu'un autre volume nous attend derri�re
			if(!faceInfo || ((faceInfo->faceEncombrement || (!(faceInfo->faceMaterial->doubleSidedMaterialEffect) && doInvertNormal)) && configurationP.currentTetra->voisins[configurationP.nextModelIntersection.idface]))
			{
				TraverserTetra(configurationP,collisionResolution);
			}else{
			#else
			faceInfo=&sceneMesh->pfaces[configurationP.nextModelIntersection.idface];
			///////////////////////////////////
			// Test de passage d'un milieu libre � un milieu encombr� (et inversement)
			if(!faceInfo->faceEncombrement)
			{
			#endif

				//On stocke le materiau dans la variable materialInfo
				t_Material_BFreq* materialInfo=&(*faceInfo).faceMaterial->matSpectrumProperty[configurationP.frequenceIndex];


				bool transmission=false;
				//Tirage al�atoire pour le test d'absorption
				if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_CHAMP_DIRECT))
				{
					//Particule absorb�e
					if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
						configurationP.stateParticule=PARTICULE_STATE_ABS_SURF;
					configurationP.energie=0.f;
					return;
				}else{
					if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_ENERGY_CALCULATION_METHOD))
					{
						//Methode �n�rg�tique, particule en collision avec la paroi
						//Particule courante = (1-alpha)*epsilon
						//Si l'absorption est totale la particule est absorb�e si tau=0
						if(materialInfo->absorption==1) //Pas de duplication possible de la particule (forcement non r�fl�chie)
						{
							if(!materialInfo->dotransmission || !(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_TRANSMISSION)))
							{
								if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
									configurationP.stateParticule=PARTICULE_STATE_ABS_SURF;
								configurationP.energie=0;
								return;
							}else{
								transmission=true;
								configurationP.energie*=materialInfo->tau;
							}
						}else{
							if(materialInfo->absorption!=0) //Pas de duplication possible de la particule (forcement r�fl�chie)
							{
								if(materialInfo->dotransmission && materialInfo->tau!=0 && configurationP.energie*materialInfo->tau>configurationP.energie_epsilon && (*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_TRANSMISSION)))
								{
									//On va dupliquer la particule
									CONF_PARTICULE configurationPTransmise=configurationP;
									configurationPTransmise.energie*=materialInfo->tau;
									bool localcolres;
									TraverserTetra(configurationPTransmise,localcolres);
									//configurationPTransmise.currentTetra=configurationP.currentTetra->voisins[configurationP.nextModelIntersection.idface];
									if(configurationPTransmise.energie>configurationPTransmise.energie_epsilon)
									{
										confEnv.duplicatedParticles.push_back(configurationPTransmise);
										/*
										reportTool->SaveParticule();
										reportTool->NewParticule(configurationPTransmise);
										Run(configurationPTransmise);
										reportTool->SaveParticule();
										reportTool->NewParticule(configurationP);
										*/
									}
								}
								configurationP.energie*=(1-materialInfo->absorption);
							} //else reflexion sans absorption
						}
					}else{
						//Test d'absorption en al�atoire
						if(GetRandValue()<=materialInfo->absorption)
						{
							// Particule non r�fl�chie
							if((*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_TRANSMISSION)) && materialInfo->dotransmission && configurationP.currentTetra->voisins[configurationP.nextModelIntersection.idface] && GetRandValue()*materialInfo->absorption<=materialInfo->tau)
							{
								transmission=true;
							}else{
								//Particule absorb�e
								if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
									configurationP.stateParticule=PARTICULE_STATE_ABS_SURF;
								configurationP.energie=0.;
								return;
							}
						}
					}
				}
				if(configurationP.energie<=configurationP.energie_epsilon)
				{
					if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
						configurationP.stateParticule=PARTICULE_STATE_ABS_SURF;
					return;
				}
				//Si Transmission on traverse la paroi
				if(transmission)
				{
					TraverserTetra(configurationP,collisionResolution);
				}else{
					// Choix de la m�thode de reflexion en fonction de la valeur de diffusion
					vec3 nouvDirection;
					vec3 faceNormal;
					if (!doInvertNormal)
						faceNormal = -faceInfo->normal;
					else
						faceNormal = faceInfo->normal;

					//Get direction for diffuse or specular part based on material info
					if(materialInfo->diffusion==1 || GetRandValue()<materialInfo->diffusion)
					{
						nouvDirection=ReflectionLaws::SolveDiffusePart(configurationP.direction,*materialInfo, faceNormal,configurationP);
					}else{
						nouvDirection=ReflectionLaws::SolveSpecularPart(configurationP.direction, *materialInfo, faceNormal, configurationP);
					}

					//Calcul de la nouvelle direction de r�flexion (en reprenant la c�l�rit� de propagation du son)
					configurationP.direction=nouvDirection*distanceSurLePas;
					collisionResolution=true;
					SetNextParticleCollision(configurationP);
				}
			}
		}

		if(iteration>1000)
		{
			//Elle est d�truite et l'utilisateur en sera inform�
			if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
				configurationP.stateParticule=PARTICULE_STATE_LOOP;
			configurationP.energie=0;
			return;
		}
	}
	if(configurationP.elapsedTime==0.f)
	{   //Aucune collision sur le pas de temps courant
		FreeParticleTranslation(configurationP,configurationP.direction);
	}else{
		//Il y a eu une ou plusieurs collisions sur le pas de temps courant
		FreeParticleTranslation(configurationP,configurationP.direction*((deltaT-configurationP.elapsedTime)/deltaT));
		configurationP.elapsedTime=0; //remise du compteur � 0
	}
}


void CalculationCore::TraverserTetra(CONF_PARTICULE &configurationP, bool& collisionResolution)
{
	// R�cuperation de l'information de la face
	t_cFace* faceInfo = configurationP.currentTetra->faces[configurationP.nextModelIntersection.idface].face_scene;
	//test de passage d'un t�tra�dre � un autre
	//PASSAGE DE TETRAEDRE

	if(!configurationP.currentTetra->voisins[configurationP.nextModelIntersection.idface])
	{
		#ifdef _DEBUG
		std::cout<<"La particule va sortir du perim�tre du volume car une face du domaine est mal orient�e ou le maillage est incorrect. La particule a �t� supprim�e";
		#endif
		configurationP.energie=0;
		if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
			configurationP.stateParticule=PARTICULE_STATE_LOST;
		return;
	}else{
		reportTool->ParticuleGoToNextTetrahedra(configurationP,configurationP.currentTetra->voisins[configurationP.nextModelIntersection.idface]);
		t_Tetra* oldTetra=configurationP.currentTetra;
		//Affectation du nouveau volume
		configurationP.currentTetra=configurationP.currentTetra->voisins[configurationP.nextModelIntersection.idface];
		//Calcul de la prochaine collision
		SetNextParticleCollision(configurationP);
		collisionResolution=true; //On refait un test de collision avec les tetrahedres

		//Si la particule passe d'un volume d'encombrement � un autre type de volume (faceInfo appartient � l'ancien volume)
		if(*configurationTool->FastGetConfigValue(Core_Configuration::IPROP_DO_CALC_ENCOMBREMENT) && faceInfo && oldTetra->volumeEncombrement!=configurationP.currentTetra->volumeEncombrement)
			SetNextParticleCollisionWithObstructionElement(configurationP);
	}
}

void CalculationCore::OnChangeCelerite(CONF_PARTICULE &configurationP, t_Tetra* tetra2)
{
	double c1=configurationP.direction.length();
	double c2=configurationTool->GetNormVecPart(configurationP.position,tetra2);
	double xy_length=vec3(configurationP.direction.x,configurationP.direction.y,0).length();
	double cos_gamma1=xy_length/c1;
	double cos_phi=configurationP.direction.x/xy_length;
	double sin_phi=configurationP.direction.y/xy_length;
	double cos_gamma2=(cos_gamma1*c2)/c1;

	short sgn=INTSIGN(configurationP.direction.z);
	if(cos_gamma2>1)
	{
		cos_gamma2=1-EPSILON;
		sgn=-sgn;
	}
	double sin_gamma2=sqrt(1-pow(cos_gamma2,2));

	configurationP.direction.x=c2*cos_gamma2*cos_phi;
	configurationP.direction.y=c2*cos_gamma2*sin_phi;
	configurationP.direction.z=sgn*c2*sin_gamma2;
}


entier_court  CalculationCore::GetTetraFaceCollision(CONF_PARTICULE &configurationP, vec3 &translationVector, float &t)
{
	vec3 dir=translationVector;
	float* posPart=configurationP.position;
	float* dirPart=dir;
	float u,v;
	vec3* scenenodes=sceneTetraMesh->nodes;
	ivec3 tmpsmt;
	ivec3& sommetsFace=tmpsmt;
	if(configurationP.currentTetra)
	{

		//////////////
		// Test de passage a un autre t�tra�dre
		TetraFaceTest(0);
		TetraFaceTest(1);
		TetraFaceTest(2);
		TetraFaceTest(3);

		//D� � une marge d'erreur aucune collision sur les faces du t�trah�dre n'a pu �tre trouv�
		// Cette marge d'erreur d�pend du maillage du volumes et de la position d'origine des particules

		//configurationP.outputToParticleFile=true; //debug
		//On recherche parmis les t�tra�dres voisins(ceux o� la particule est �galement sur la surface du t�tra�dre), lequel est capable de positionner la prochaine face
		//de collision.
		//Si il est toujours impossible de positionner la particule, celle-ci sera d�truite
		t_Tetra* old_tetra=configurationP.currentTetra;
		for(unsigned short idtet=0;idtet<4;idtet++)
		{
			if(old_tetra->voisins[idtet]!=NULL)
			{
				configurationP.currentTetra=old_tetra->voisins[idtet];
				if(core_mathlib::DotInTetra(configurationP.position,this->sceneTetraMesh->nodes[configurationP.currentTetra->sommets.a],
					this->sceneTetraMesh->nodes[configurationP.currentTetra->sommets.b],
					this->sceneTetraMesh->nodes[configurationP.currentTetra->sommets.c],
					this->sceneTetraMesh->nodes[configurationP.currentTetra->sommets.d]))
				{
					TetraFaceTest(0);
					TetraFaceTest(1);
					TetraFaceTest(2);
					TetraFaceTest(3);
				}
			}
		}
		configurationP.energie=0;
		if(configurationP.stateParticule==PARTICULE_STATE_ALIVE)
			configurationP.stateParticule=PARTICULE_STATE_LOST;
	}
	return -1;
}

#ifndef UTILISER_MAILLAGE_OPTIMISATION
void CalculationCore::SetNextParticleCollision(CONF_PARTICULE &configurationP)
{
	INTERSECTION_INFO collisionInfos;
	float minDist=9999999999.f;
	for(uentier cface=0;cface<sceneMesh->pface_size;cface++)
	{
		float faceDist;
		if(CollisionTest(configurationP,cface,collisionInfos,faceDist))
		{
			if(faceDist<minDist)
			{
				float dotVal=configurationP.direction.dot(sceneMesh->pfaces[collisionInfos.idface].normal);
				if(dotVal>0) //si la face est orient� vers nous
				{
					configurationP.nextModelIntersection=collisionInfos;
					minDist=faceDist;
				}
			}
		}
	}
}

void CalculationCore::FreeParticleTranslation(CONF_PARTICULE &configurationP,const vec3 &translationVector)
{
	if(configurationP.currentTetra)
	{
		bool changeTetrahedra=true;
		float tMax=-1;
		while(changeTetrahedra) //On navigue de tetraedre a tetraedre jusqu'a ce que le point de collision soit plus loin que le vecteur de translation
		{
			changeTetrahedra=false;
			float t;
			entier_court idfacemaxdist=GetTetraFaceCollision(configurationP,translationVector,t);
			if(idfacemaxdist>=0 && t<1 && t>=-EPSILON)
			{
				if(configurationP.currentTetra->voisins[idfacemaxdist] && tMax<t) //
				{
					reportTool->ParticuleGoToNextTetrahedra(configurationP,configurationP.currentTetra->voisins[idfacemaxdist]);
					configurationP.currentTetra=configurationP.currentTetra->voisins[idfacemaxdist];
					changeTetrahedra=true;
					tMax=t;
				}
			}
		}
	}
	if(configurationP.currentTetra->linkedRecepteurP)
		reportTool->ParticuleFreeTranslation(configurationP,configurationP.position+translationVector);

	//////////////
	// Translation de la particule
	configurationP.position+=translationVector;
}

bool CalculationCore::CollisionTest(CONF_PARTICULE &configurationP,uentier &faceIndex,INTERSECTION_INFO &infoIntersection, float &factDistance)
{
	using namespace std;

	float t,u,v;

	vec3 *vert0=&sceneMesh->pvertices[sceneMesh->pfaces[faceIndex].sommets.a];
	vec3 *vert1=&sceneMesh->pvertices[sceneMesh->pfaces[faceIndex].sommets.b];
	vec3 *vert2=&sceneMesh->pvertices[sceneMesh->pfaces[faceIndex].sommets.c];


	if(collision_manager::intersect_triangle(configurationP.position,configurationP.direction,
		*vert0,
		*vert1,
		*vert2,
		&t,&u,&v)==1)
	{
		if(t>=-BARELY_EPSILON) //t correspond au nombre de pas de temps pour atteindre la collision
		{
			factDistance=t;
			infoIntersection.collisionPosition=configurationP.position+configurationP.direction*t;
			infoIntersection.idface=faceIndex;
			infoIntersection.percCollision=t;
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
}

#else
void CalculationCore::SetNextParticleCollision(CONF_PARTICULE &configurationP)
{
	//INTERSECTION_INFO collisionInfos;
	float t;
	configurationP.nextModelIntersection.idface=GetTetraFaceCollision(configurationP,configurationP.direction,t);

	configurationP.nextModelIntersection.collisionPosition=configurationP.position+configurationP.direction*t;
}

bool CalculationCore::VisabilityTest(CONF_PARTICULE &configurationP, vec3 &TargetPosition)
{
	float obst_dist;
	float t;
	float rec_dist = (configurationP.position - TargetPosition).length();

	configurationP.nextModelIntersection.idface = GetTetraFaceCollision(configurationP, configurationP.direction, t);
	configurationP.nextModelIntersection.collisionPosition = configurationP.position + configurationP.direction*t;

	obst_dist = configurationP.direction.length()*t;

	CONF_PARTICULE testParticle = configurationP;

	int maxIteration = 5000;
	int iteration = 0;

	while (testParticle.currentTetra->faces[testParticle.nextModelIntersection.idface].face_scene == NULL && iteration++<maxIteration)
	{
		testParticle.position = testParticle.nextModelIntersection.collisionPosition;
		testParticle.currentTetra = testParticle.currentTetra->voisins[testParticle.nextModelIntersection.idface];

		testParticle.nextModelIntersection.idface = GetTetraFaceCollision(testParticle, testParticle.direction, t);
		testParticle.nextModelIntersection.collisionPosition = testParticle.position + testParticle.direction*t;
		obst_dist += testParticle.direction.length()*t;

		if (rec_dist < obst_dist)
		{
			return true;
		}
	}

	return false;
}

void CalculationCore::FreeParticleTranslation(CONF_PARTICULE &configurationP,const vec3 &translationVector)
{
	reportTool->ParticuleFreeTranslation(configurationP,configurationP.position+translationVector);
	// On prend en compte le rapprochement vers l'encombrement virtuel
	if(configurationP.currentTetra->volumeEncombrement)
		configurationP.distanceToNextEncombrementEle-=translationVector.length();
	configurationP.position+=translationVector;
}
#endif

