/* ----------------------------------------------------------------------
* I-SIMPA (http://i-simpa.ifsttar.fr). This file is part of I-SIMPA.
*
* I-SIMPA is a GUI for 3D numerical sound propagation modelling dedicated
* to scientific acoustic simulations.
* Copyright (C) 2007-2014 - IFSTTAR - Judicael Picaut, Nicolas Fortin
*
* I-SIMPA is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* (at your option) any later version.
* 
* I-SIMPA is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA or 
* see <http://ww.gnu.org/licenses/>
*
* For more information, please consult: <http://i-simpa.ifsttar.fr> or 
* send an email to i-simpa@ifsttar.fr
*
* To contact Ifsttar, write to Ifsttar, 14-20 Boulevard Newton
* Cite Descartes, Champs sur Marne F-77447 Marne la Vallee Cedex 2 FRANCE
* or write to scientific.computing@ifsttar.fr
* ----------------------------------------------------------------------*/

#include "first_header_include.hpp"

#include "data_manager/tree_core/e_core_core.h"

#ifndef __E_CORE_SPPS__
#define __E_CORE_SPPS__

/*! \file e_core_sppscore.h
    \brief Element correspondant au moteur de calcul "Simulation de la propagation de particules sonores"
*/
enum COMPUTATION_METHOD
{
	COMPUTATION_METHOD_ALEATOIRE,
	COMPUTATION_METHOD_ENERGETIQUE
};

/**
	\brief Element correspondant au moteur de calcul "Simulation de la propagation de particules sonores"
*/
class E_Core_Spps: public E_Core_Core
{
protected:
	void InitTransmission(E_Core_Core_Configuration* confCore)
	{
		confCore->AppendPropertyDecimal("trans_epsilon",wxTRANSLATE("Limite de transmission"),5,true,1,true,true,10,0,true);
		confCore->AppendPropertyBool("trans_calc",wxTRANSLATE("Calcul Transmission"),true,true);
		confCore->AppendPropertyDecimal("rayon_recepteurp",wxTRANSLATE("Rayon des récepteurs ponctuels"),.31f,false,5,false,true,0,EPSILON,true);
	}
	void InitSurfaceReceiverMethod(E_Core_Core_Configuration* confCore)
	{
		std::vector<wxString> surfMethod;
		std::vector<int> surfMethodIndex;
		surfMethod.push_back(wxTRANSLATE("Cartographie intensité"));
		surfMethodIndex.push_back(0);
		surfMethod.push_back(wxTRANSLATE("Cartographie SPL"));
		surfMethodIndex.push_back(1);
		confCore->AppendPropertyList("surf_receiv_method",wxTRANSLATE("Export des récepteurs surfacique de type"),surfMethod,0,false,1,surfMethodIndex,true);
	}
	void InitNewProperties() //Nouvelle proprietes 07/04/2009
	{
		this->AppendPropertyText("stats_filename","stats","statsSPPS.gabe",true,true)->Hide();
		this->AppendPropertyText("intensity_folder","intensity_folder",wxTRANSLATE("IntensityAnimation"),true,true)->Hide();
		this->AppendPropertyText("intensity_filename","intensity_filename","intensity.rpi",true,true)->Hide();
		this->AppendPropertyText("intensity_rp_filename","intensity_rp_filename","ponct_intensity.gabe",true,true)->Hide();
		#if 0
		    // Code source à destination de PoEdit
			_("Intensity Animation");
			_("Core Statistics");
			_("Sound level per source");
		#endif
	}
	void InitExportRs(Element* confCore)
	{
		confCore->AppendPropertyBool("output_recs_byfreq","Export des récepteurs de surface par bande de fréquence",true,true);
		_("Export surface receivers for each frequency band");
	}
	void InitOutputRecpBySource(Element* confCore) {
		confCore->AppendPropertyBool("output_recp_bysource", "Echogramme par source", false, true);
	}
	void InitRandomSeed(Element* confCore) {
		confCore->AppendPropertyEntier("random_seed","Random seed", 0,true, false, true);
		#if 0
		    // Code source à destination de PoEdit
			_("Random number generator initialization");
		#endif
	}
	void InitDiffusion_order(Element* confCore) {
		confCore->AppendPropertyEntier("diffusion_order","Diffusion order",0,true,false,true,0,0);	
	}
	void InitDiffusion_when_reached(Element* confCore) {
		confCore->AppendPropertyBool("specular_when_order_reached","Specular when order reached",false,true);
	}
public:

	E_Core_Spps( Element* parent, wxXmlNode* noeudCourant)
		:E_Core_Core(parent,"SPPS",ELEMENT_TYPE_CORE_SPPS,noeudCourant)
	{
		SetIcon(GRAPH_STATE_EXPANDED,GRAPH_SPPSCORE_OPEN);
		SetIcon(GRAPH_STATE_NORMAL,GRAPH_SPPSCORE_CLOSE);
		E_Core_Core_Configuration* confCore=dynamic_cast<E_Core_Core_Configuration*>(this->GetElementByType(ELEMENT_TYPE_CORE_CORE_CONFIG));
		if(confCore)
		{
			if(!confCore->IsPropertyExist("trans_epsilon")) //mise à jour projet < 12/11/2008
				InitTransmission(confCore);
			Element* proptodel=NULL;
			if(confCore->IsPropertyExist("outpout_recs_byfreq",&proptodel))//mise à jour projet < 10/04/2009
			{
				confCore->DeleteElementByXmlId(proptodel->GetXmlId());
			}
			if(!confCore->IsPropertyExist("surf_receiv_method"))
				InitSurfaceReceiverMethod(confCore);
			if(!confCore->IsPropertyExist("output_recp_bysource")) {
				InitOutputRecpBySource(confCore);
			}
			if(!confCore->IsPropertyExist("random_seed")) {
				InitRandomSeed(confCore);
			}
			if(!confCore->IsPropertyExist("diffusion_order")) {
				InitDiffusion_order(confCore);
			}
			if(!confCore->IsPropertyExist("specular_when_order_reached")) {
				InitDiffusion_when_reached(confCore);
			}
			InitExportRs(confCore);
		}
		InitNewProperties();
	}

	E_Core_Spps( Element* parent)
		:E_Core_Core(parent,"SPPS",ELEMENT_TYPE_CORE_SPPS)
	{

		SetIcon(GRAPH_STATE_EXPANDED,GRAPH_SPPSCORE_OPEN);
		SetIcon(GRAPH_STATE_NORMAL,GRAPH_SPPSCORE_CLOSE);
		this->AppendFilsByType(ELEMENT_TYPE_CORE_CORE_CONFMAILLAGE);
		E_Core_Core_Configuration* confCore=new E_Core_Core_Configuration(this);
		this->AppendFils(confCore);

		confCore->InitProperties();
		InitNewProperties();
		InitExportRs(confCore);
		InitSurfaceReceiverMethod(confCore);
		InitOutputRecpBySource(confCore);
		InitRandomSeed(confCore);
		InitDiffusion_order(confCore);
		InitDiffusion_when_reached(confCore);
		//Ajout des propriétés propres à spps
		std::vector<wxString> computationMethods;
		std::vector<int> computationMethodsIndex;
		computationMethods.push_back(_("Random"));
		computationMethods.push_back(_("Energetic"));

		confCore->AppendPropertyEntier("nbparticules","Particules par source",150000,true,false,true,0,1);
		confCore->AppendPropertyEntier("nbparticules_rendu","Particules par source (rendu)",0,true,false,true,0,0);
		confCore->AppendPropertyBool("abs_atmo_calc","Calcul Absorption atmosphérique",true,true);
		confCore->AppendPropertyBool("enc_calc","Calcul Encombrement",true,true);
		confCore->AppendPropertyBool("direct_calc","Calcul du Champ Direct uniquement",false,true);
		confCore->AppendPropertyList("computation_method","Méthode de calcul",computationMethods,0,false,1,computationMethodsIndex,true);

		InitTransmission(confCore);

		#if 0
		    // Code source à destination de PoEdit
			_("SPPS");
			_("Random");
			_("Energetic");
			_("Export surface receivers for each frequency band");
			_("Calculation method");
			_("Number of sound particles per source");
			_("Number of sound particles per source (display)");
			_("Active calculation of atmospheric absorption");
			_("Active calculation of diffusion by fitting objects");
			_("Active calculation of direct field only");
			_("Radius of receivers (m)");
			_("Limit of propagation (10^n)");
			_("Active calculation of acoustic transmission");
			_("Punctual intensity");
		#endif

		this->AppendFils(new E_Core_Core_Bfreqselection(this));

		this->AppendPropertyText("modelName","","mesh.cbin",true,true)->Hide();
		this->AppendPropertyText("exeName","","sppsNantes.exe")->Hide();
		this->AppendPropertyText("corePath","",wxString("sppsNantes")+wxFileName::GetPathSeparator())->Hide();
		this->AppendPropertyText("tetrameshFileName","","tetramesh.mbin",true,true)->Hide();
	}
	wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* thisNode = E_Core_Core::SaveXMLDoc(NoeudParent);
		thisNode->SetName("spps");
		return thisNode;
	}
	void Modified(Element* eModif)
	{
		Element* elConf=this->GetElementByType(ELEMENT_TYPE_CORE_CORE_CONFIG);
		if(elConf)
		{
			t_elementInfo filsInfo=eModif->GetElementInfos();
			if(filsInfo.libelleElement=="nbparticules" && eModif->GetElementParent()->IsPropertyExist("nbparticules_rendu"))
			{
				int nbpart=elConf->GetEntierConfig("nbparticules");
				int nbpartrendu=elConf->GetEntierConfig("nbparticules_rendu");
				if(nbpart<nbpartrendu)
					elConf->UpdateEntierConfig("nbparticules_rendu",nbpart);
			}else if(filsInfo.libelleElement=="nbparticules_rendu" && elConf->IsPropertyExist("nbparticules"))
			{
				int nbpart=elConf->GetEntierConfig("nbparticules");
				int nbpartrendu=elConf->GetEntierConfig("nbparticules_rendu");
				if(nbpart<nbpartrendu)
					elConf->UpdateEntierConfig("nbparticules",nbpartrendu);
				if(nbpartrendu>0)
				{
					unsigned int nbpasdetemps=elConf->GetDecimalConfig("duree_simulation")/elConf->GetDecimalConfig("pasdetemps");
					unsigned int total_data=nbpartrendu*nbpasdetemps*sizeof(float)*4*ApplicationConfiguration::GLOBAL_CURRENT_APPLICATION_INFORMATIONS.quant_Sources_Actives;
					wxLogWarning("La taille estimée du fichier de particule par bande de fréquence est de %.2f Mo",float(total_data)/pow(10.f,6.f));
				}
			}else if(filsInfo.libelleElement=="computation_method")
			{
				elConf->SetReadOnlyConfig("trans_epsilon",!elConf->GetListConfig("computation_method")==COMPUTATION_METHOD_ENERGETIQUE);
				elConf->SetReadOnlyConfig("diffusion_order",!elConf->GetListConfig("computation_method")==COMPUTATION_METHOD_ENERGETIQUE);
				elConf->UpdateEntierConfig("diffusion_order",0);
				elConf->SetReadOnlyConfig("specular_when_order_reached",!elConf->GetListConfig("computation_method")==COMPUTATION_METHOD_ENERGETIQUE);
			}
		}
		Element::Modified(eModif);
	}

	wxXmlNode* SaveXMLCoreDoc(wxXmlNode* NoeudParent)
	{
		wxXmlNode* NoeudCourant=E_Core_Core::SaveXMLCoreDoc(NoeudParent);
		NoeudCourant->AddProperty("particules_directory",ApplicationConfiguration::CONST_REPORT_PARTICLE_FOLDER_PATH);
		NoeudCourant->AddProperty("particules_filename",ApplicationConfiguration::CONST_REPORT_PARTICLE_FILENAME);
		return NoeudCourant;
	}
};

#endif
