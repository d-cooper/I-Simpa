# -*- coding: cp1252 -*-

import uictrl as ui
from libsimpa import *


def GetMixedLevel(folderwxid):
    """
     Retourne un tableau contenant le niveau sonore global et toute bande des r�cepteurs ponctuels d'un dossier
     folderwxid identifiant wxid de l'�l�ment dossier contenant les r�cepteurs ponctuels.
    """
    cols=[]
    #folder devient l'objet dossier
    folder=ui.element(folderwxid)
    #dans un tableau on place les indices des fichiers de donn�es des r�cepteurs ponctuels
    recplist=folder.getallelementbytype(ui.element_type.ELEMENT_TYPE_REPORT_GABE_RECP)
    #Pour chaque r�cepteur on demande � l'application les donn�es trait�es du fichier (niveau sonore et cumuls)
    for idrecp in recplist:
        #recp devient l'objet ayant comme indice idrecp (entier)
        recp=ui.element(idrecp)
        if recp.getinfos()["name"]=="Sound level":
            #on demande le calcul des param�tres sonores
            ui.application.sendevent(recp,ui.idevent.IDEVENT_RECP_COMPUTE_ACOUSTIC_PARAMETERS,{"TR":"15;30", "EDT":"", "D":""})
            #on recupere l'element parent (le dossier de r�cepteur ponctuel)
            pere=ui.element(recp.getinfos()["parentid"])
            #application.sendevent(pere,idevent.IDEVENT_RELOAD_FOLDER)
            nomrecp=pere.getinfos()["label"]
            #on recupere les donn�es calcul�es
            params=ui.element(pere.getelementbylibelle('Acoustic parameters'))
            #on stocke dans gridspl le tableau des niveaux de pression
            gridparam=ui.application.getdataarray(params)
            #on ajoute la colonne
            if len(cols)==0: #si le tableau de sortie est vide alors on ajoute les libell�s des lignes
                cols.append(list(zip(*gridparam)[0])) #libell� Freq et Global
            idcol=gridparam[0].index("EDT (s)") #Changer le param�tre par celui pour lequel on veut la fusion
            cols.append([nomrecp]+list(zip(*gridparam)[idcol][1:])) #1ere colonne, (0 etant le libell� des lignes) et [1:] pour sauter la premiere ligne 
    return cols

def SaveLevel(tab,path):
    #Creation de l'objet qui lit et ecrit les fichiers gabe
    gabewriter=Gabe_rw(len(tab))
    labelcol=stringarray()
    for cell in tab[0][1:]:
        labelcol.append(cell.encode('cp1252'))
    for col in tab[1:]:
        datacol=floatarray()
        for cell in col[1:]:
            datacol.append(float(cell))
        gabewriter.AppendFloatCol(datacol,str(col[0]))
    gabewriter.Save(path.encode('cp1252'))
    
def dofusion(folderwxid, path):
    arraydata=GetMixedLevel(folderwxid)
    SaveLevel(zip(*arraydata),path)
    #raffraichie l'arbre complet
    ui.application.sendevent(ui.element(ui.element(ui.application.getrootreport()).childs()[0][0]),ui.idevent.IDEVENT_RELOAD_FOLDER)

class manager:
    def __init__(self):
        self.GetMixedLevelid=ui.application.register_event(self.OnFusion)
    def getmenu(self,typeel,idel,menu):
        el=ui.element(idel)
        infos=el.getinfos()
        if infos["name"]==u"Punctual receivers":
            menu.insert(0,())
            menu.insert(0,(u"Merge point receivers",self.GetMixedLevelid))
            return True
        else:
            return False
    def OnFusion(self,idel):
        grp=ui.e_file(idel)
        dofusion(idel,grp.buildfullpath()+"fusion.gabe")
ui.application.register_menu_manager(ui.element_type.ELEMENT_TYPE_REPORT_FOLDER, manager())
