# -*- coding: cp1252 -*-

import uictrl as ui
import libsimpa as ls

# D�claration de la m�thode de traduction en cas d'erreur d'import du dictionnaire
def _(msg):
    return msg

try:
    import uilocale
    #Construction du chemin du script
    ScriptFolder=ui.application.getapplicationpath()["userscript"]+"recp_res_norm\\"
    #D�claration de la m�thode de traduction
    _=uilocale.InstallUiModule(ScriptFolder,ui.application.getlocale())
except:
    #En cas d'erreur afficher l'erreur et continuer
    import sys, traceback
    print("Unable to import the language dictionary !")
    traceback.print_exc(file=sys.stdout)

def MakeSPLNormalGrid(folderwxid, save_as):
    #Dictionnaire qui contiendra les donn�es pour tout les r�cepteurs
    global_dict={}
    #folder devient l'objet dossier
    folder=ui.element(folderwxid)
    #dans un tableau on place les indices des fichiers de donn�es des r�cepteurs ponctuels
    recplist=folder.getallelementbytype(ui.element_type.ELEMENT_TYPE_REPORT_GABE_RECP)
    #Pour chaque fichiers de type ELEMENT_TYPE_REPORT_GABE_RECP
    for idrecp in recplist:
        #recp devient l'objet ayant comme indice idrecp (entier)
        recp=ui.element(idrecp)
        #Stocke les informations sur cet �l�ment
        recpinfos=recp.getinfos()
        #Si cet �l�ment est le niveau sonore
        if recpinfos["name"]=="soundpressure":
            #Lecture du libell� du dossier parent
            ponc_receiver_name=ui.element(recpinfos["parentid"]).getinfos()["label"]
            #Lecture des informations trait� par I-SIMPA. C'est � dire les niveaux directement en dB avec les colonnes de cumul
            gridspl=ui.application.getdataarray(recp)
            #Lecture du cumul sur les pas de temps et sur les fr�quences
            global_spl=gridspl[-1][-1]
            #Enregistrement dans le dictionnaire
            global_dict[ponc_receiver_name]=global_spl
    ###
    #Interface de s�lection du r�cepteur ponctuel de r�f�rence
    #Cr�ation de la liste des libell�s des r�cepteurs ponctuels
    sorted_pr_labels=global_dict.keys()
    #Tri des libell�s
    sorted_pr_labels.sort()
    #Cr�ation des champs � afficher � l'utilisateur
    lbl_pr_dialog=_("Ponctual receiver name")
    choices={lbl_pr_dialog : sorted_pr_labels}
    #Affichage de la fen�tre de dialogue
    user_choice=ui.application.getuserinput("Normalisation tool","Please choose the reference data.",choices)
    #Si l'utilisateur a valid�
    if user_choice[0]:
        #Lecture du libell� du r�cepteur de r�f�rence
        pr_reference=user_choice[1][lbl_pr_dialog]
        sub_by=global_dict[pr_reference]
        ##
        # Cr�ation de l'objet d'�criture du format de grille de donn�es. Le nombre de colonnes correspond au nombre de r�cepteurs ponctuels et une colonne de libell�
        out_grid=ls.Gabe_rw(len(global_dict.keys())+1)
        #Cr�ation d'un vecteur de chaine de caract�res
        labels_vector=ls.stringarray()
        #Ajout d'une ligne
        labels_vector.append(_("Global").encode("cp1252"))
        #Ajout de la colonne de libell�
        out_grid.AppendStrCol(labels_vector,"")
        ##
        #Calcul du facteur de normalisation pour chaque r�cepteur ponctuel
        for pr_name in sorted_pr_labels:
            #Ajout d'une colonne par r�cepteur
            #Cr�ation d'un vecteur de nombre
            spl_vector=ls.floatarray()
            spl_vector.append(global_dict[pr_name]-sub_by)
            #Ajout de la colonne
            out_grid.AppendFloatCol(spl_vector,pr_name,3)
    
        #L'utilisateur ne pourra pas modifier la feuille de donn�es
        out_grid.SetReadOnly()
        #Sauvegarde de la grille
        out_grid.Save(save_as)
        return True
    return False
#D�claration de la classe qui contiendra l'indice de l'outil de normalisation
class manager:
    def __init__(self):
        #On enregistre la m�thode afin d'obtenir l'indice � ajouter au menu popup
        self.MakeSPLNormalGridEventId=ui.application.register_event(self.OnMakeSPLNormalGrid)
    #Cette m�thode sera ex�cut�e lors-ce que l'utilisateur clique bouton droit sur un dossier de r�sultat
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if infos["name"]=="R�cepteurs_Ponctuels":
            menu.insert(0,(_("Normalise SPL"),self.MakeSPLNormalGridEventId))
            return True
        else:
            return False
    def OnMakeSPLNormalGrid(self,idel):
        grp=ui.e_file(idel)
        if MakeSPLNormalGrid(idel,(grp.buildfullpath().decode('cp1252')+_("SPL_norm")+".gabe").encode("cp1252")):
            #raffraichie l'arbre de r�sultat
            ui.application.sendevent(grp,ui.idevent.IDEVENT_RELOAD_FOLDER)
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())
            
