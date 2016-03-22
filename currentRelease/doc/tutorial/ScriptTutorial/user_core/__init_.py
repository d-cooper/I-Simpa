# -*- coding: cp1252 -*-
import uictrl as ui

# D�claration de la m�thode de traduction en cas d'erreur d'import du dictionnaire
def _(msg):
    return msg

try:
    import uilocale
    #Construction du chemin du script
    ScriptFolder=ui.application.getapplicationpath()["userscript"]+"user_core\\"
    #D�claration de la m�thode de traduction
    _=uilocale.InstallUiModule(ScriptFolder,ui.application.getlocale())
except:
    #En cas d'erreur afficher l'erreur et continuer
    import sys, traceback
    print("Unable to import the language dictionary !")
    traceback.print_exc(file=sys.stdout)


class user_core(ui.element):
    """
      Code de calcul utilisateur
    """
    def __init__(self,idel):
		#Initialisation de l'�l�ment
        ui.element.__init__(self,idel)
        
        if not self.hasproperty("exeName"): #La propri�t� n'existe pas s'il s'agit d'un nouveau projet, elle existe si le projet est charg�
            #Cr�ation des param�tres du maillage tetgen, ne pas le faire si le calcul n'a pas besoin de maillage t�tra�drique
            self.appendfilsbytype(ui.element_type.ELEMENT_TYPE_CORE_CORE_CONFMAILLAGE)
			#Ajout de la s�lection des bandes de fr�quences, ne pas le faire si c'est inutile
            self.appendfilsbytype(ui.element_type.ELEMENT_TYPE_CORE_CORE_BFREQSELECTION)
			#Ajout du noeud de configuration, qui contient par d�faut les propri�t�s de pas de temps et de dur�e de simulation
            coreconf=ui.element(self.appendfilsbytype(ui.element_type.ELEMENT_TYPE_CORE_CORE_CONFIG))
			####
			#Ces propri�t�s sont n�cessaire pour que I-SIMPA connaisse les fichiers relatifs au code de calcul
			#Nom et format du mod�le 3D
            ui.element(self.appendpropertytext("modelName","","mesh.cbin",True,True)).hide()
			#Nom du maillage t�tra�drique
            ui.element(self.appendpropertytext("tetrameshFileName","","tetramesh.mbin",True,True)).hide()
			#Nom et type (exe,py ou pyc) du fichier executable
            ui.element(self.appendpropertytext("exeName","","user_core.exe")).hide()
            ui.element(self.appendpropertytext("corePath","","usercore\\")).hide()
            
            #Cr�ation des param�tres de calculs
            coreconf.appendpropertylist("solver_mode",_("Mode de calcul"),[[_("Temporel"),_("Stationnaire")],[0,1]],0,False,1,True)
        else:
            #Chargement d'un projet existant
            pass
    def gettreelabel(self):
        """
            Retourne le libell� visible dans l'arbre
        """
        return "Code de calcul utilisateur"
    def geticonid(self,state_open):
        """
            Retourne l'indice de l'icone de l'�l�ment en fonction de son �tat
        """
        if state_open:
            return ui.graph.GRAPH_FOLDER_OPEN
        else:
            return ui.graph.GRAPH_FOLDER
    ###
    # Cette m�thode est appel� par I-SIMPA lors-ce qu'un sous �l�ment du code de calcul est modifi�
    # Certains param�tres sont li�s entres eux, cette m�thode permet de modifier l'acc�s de modification selon l'�tat de la m�thode de calcul
    def modified(self,idelmodified):
        #Le mode de calcul stationnaire n'a pas besoin des param�tres de pas de temps ni de dur�e
        if ui.element(idelmodified).getinfos()["name"]=="solver_mode":
            #le mode de calcul a �t� modifi� par l'utilisateur
            elconf=ui.element(self.getelementbytype(ui.element_type.ELEMENT_TYPE_CORE_CORE_CONFIG))
            is_temporel=(elconf.getlistconfig("solver_mode")==0)
            elconf.setreadonlyconfig("duree_simulation",not is_temporel)
            elconf.setreadonlyconfig("pasdetemps",not is_temporel)
        ui.element.modified(self,idelmodified)
