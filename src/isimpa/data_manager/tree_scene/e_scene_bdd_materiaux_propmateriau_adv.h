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
#include "data_manager/element.h"

#ifndef __ELEMENT_PROPERTY_MATERIAL_ADVANCED__
#define __ELEMENT_PROPERTY_MATERIAL_ADVANCED__

/** \file e_scene_volumes.h
   \brief Dans ce dossier se trouve tout les volumes détéctés par l'interface.
*/

/**
   \brief Dans ce dossier se trouve tout les volumes détéctés par l'interface.
*/
class E_Scene_Bdd_Materiaux_PropertyMaterial_Adv: public Element
{
public:
	E_Scene_Bdd_Materiaux_PropertyMaterial_Adv( wxXmlNode* noeudCourant ,  Element* parent);

	E_Scene_Bdd_Materiaux_PropertyMaterial_Adv( Element* parent);
	
	// Sauvegarde des informations à destination des moteurs de calculs
	virtual Element* AppendFilsByType(ELEMENT_TYPE etypefils,const wxString& libelle="");
	wxXmlNode* SaveXMLCoreDoc(wxXmlNode* NoeudParent);
	//wxXmlNode* SaveXMLCoreDoc(wxXmlNode* NoeudParent);

	wxXmlNode* SaveXMLDoc(wxXmlNode* NoeudParent);
	
	void OnRightClic(wxMenu* leMenu);

	void AddCustomBRDF();
	void AddCustomBRDFSamplingMehtods();
	void AddCustomBRDFExpnentFactor();
};

#endif