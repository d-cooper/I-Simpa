﻿/* ----------------------------------------------------------------------
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

#include <wx/grid.h>
#include <wx/textctrl.h>

#ifndef __FILE_EDITOR__
#define __FILE_EDITOR__

/*! \file wxGridFileColorEditor.h
*  @brief Editeur de cellule de type fichier
*  @see E_Data_File
*/

/**
*  @brief Editeur de cellule de type fichier
*
*  Lors ce que l'utilisateur désire éditer un champ fichier l'instance de cette classe associé est appelé via wxGridCellFileEditor::BeginEdit
*  Lors ce que la grille MainPropGrid affiche ce type de champ elle appelle wxGridCellFileEditor::Show
*  A la fin de l'édition du champ la méthode wxGridCellFileEditor::EndEdit est appelé afin de mettre ŕ jour la cellule texte de base
*  @see E_Data_File
*/

class wxGridCellFileEditor : public wxGridCellTextEditor
{
private:
	wxString dialogTitle;
	wxString fileExtension;
public:
	wxGridCellFileEditor(wxString dialogTitle, wxString fileExtension);

	/**
	* Début d'édition du champ
	* @param row Ligne
	* @param col Colonne
	* @param grid Pointeur vers le contrôle
	*/
	virtual void BeginEdit(int row, int col, wxGrid* grid);

	/**
	* Affichage du champ
	* @param show "use the specified attributes to set colours/fonts for it."
	* @param attr Attributs de la cellule associée
	*/
	virtual void Show(bool show, wxGridCellAttr *attr);

	/**
	* Fin d'édition du champ
	* @param row Ligne
	* @param col Colonne
	* @param grid Pointeur vers le contrôle
	*/
	virtual bool EndEdit(int row, int col, const wxGrid *grid,
		const wxString& oldval, wxString *newval);
};

#endif