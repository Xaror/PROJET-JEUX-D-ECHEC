/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Interface;

import JeuxEchecs.Deplacement;
import JeuxEchecs.Position;
import javafx.scene.image.ImageView;

/**
 *
 * @author Bloody
 */
public class Piece {
    private String nom;
    private ImageView img;
    private String id;
    private String type;
    private String couleur;
    
    
    
    public Piece ( String nom, ImageView img, String type){
        this.nom= nom;
        this.img= img;
        this.type=type;
    }
    public String getnom(){
        return nom;
    }
    public ImageView getimg(){
        return img;
    }
    public String getid(){
        return id;
    }
    public void setid( String id){
        this.id=id;
    }
    public String gettype(){
        return type;
    }
    public void settype( String type){
        this.type=type;
    }
    public void setCouleur(String couleur){
	if ((couleur == "noir") || (couleur == "blanc"))
	this.couleur = couleur;
    }
    
    public String getCouleur(){
	return couleur;
    }
    public boolean deplacementValide(int XD,int YD,int XF,int YF, boolean e ){
	Position depart = new Position(XD , YD);
        Position arrive = new Position(XF , YF);
        Deplacement dep = new Deplacement(depart , arrive);
        boolean status = false;
        
        switch (getnom())
                {                  
                  case "Tour":
                    status = deplacementtour(dep);
                    break;
                  case "Cavalier":
                    status = deplacementcavalier(dep);  
                    break;
                  case "Fou":
                    status = deplacementfou(dep);  
                    break;
                  case "Reine":
                    status = deplacementreine(dep);  
                    break;
                  case "Roi":
                    status = deplacementroi(dep);  
                    break;
                  case "Pion":
                    status = deplacementpion(dep, e);  
                    break;
                }
            				
	return status; 
    }       
    public boolean deplacementroi(Deplacement deplacement){
        return Math.abs(deplacement.getDeplacementX()) * Math.abs(deplacement.getDeplacementY()) <= 1
            && Math.abs(deplacement.getDeplacementX()) - Math.abs(deplacement.getDeplacementY()) <= 1
            && Math.abs(deplacement.getDeplacementX()) - Math.abs(deplacement.getDeplacementY()) >= -1
            && !deplacement.deplacementNul();
    }
    public boolean deplacementpion(Deplacement dep, boolean ennemi ){
	if(dep.getDeplacementX() == 0){
            if (this.gettype().equals("noire")){ 
                return dep.getDeplacementY() <= (dep.getDepart().getLigne() == 1 ? 2 : 1) && dep.getDeplacementY() > 0;
            }
            else{
                return dep.getDeplacementY() >= (dep.getDepart().getLigne() == 6 ? -2 : -1) && dep.getDeplacementY() < 0;
                }
        }
        if((dep.getDeplacementX() == 1 || dep.getDeplacementX() == -1) && ennemi == true){
            if (this.gettype().equals("noire")){ 
                return dep.getDeplacementY() <= (dep.getDepart().getLigne() == 1 ? 1 : 1) && dep.getDeplacementY() > 0;
            }
            else{
                return dep.getDeplacementY() >= (dep.getDepart().getLigne() == 6 ? -1 : -1) && dep.getDeplacementY() < 0;
                }
        }
        return false;				
	 
    }
    public boolean deplacementcavalier(Deplacement dep){
        //return (Math.abs(deplacement.getDeplacementX() / deplacement.getDeplacementY())) == 2 | (Math.abs(deplacement.getDeplacementX() / deplacement.getDeplacementY())) == .5;
        if((dep.getDepart().getLigne() - dep.getArrivee().getLigne()== 2 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== 1) ||
           (dep.getDepart().getLigne() - dep.getArrivee().getLigne()== 2 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== -1) ||
           (dep.getDepart().getLigne() - dep.getArrivee().getLigne()== -2 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== 1) ||
           (dep.getDepart().getLigne() - dep.getArrivee().getLigne()== -2 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== -1)||
           (dep.getDepart().getLigne() - dep.getArrivee().getLigne()== 1 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== 2) ||
           (dep.getDepart().getLigne() - dep.getArrivee().getLigne()== 1 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== -2) ||
           (dep.getDepart().getLigne() - dep.getArrivee().getLigne()== -1 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== 2) ||
           (dep.getDepart().getLigne() - dep.getArrivee().getLigne()== -1 && dep.getDepart().getColonne() - dep.getArrivee().getColonne()== -2) 
            )
        {
            return true;
        }
        else 
            return false;
    }
     public boolean deplacementfou(Deplacement deplacement){
		return Math.abs(deplacement.getDeplacementX()) - Math.abs(deplacement.getDeplacementY()) == 0 && !deplacement.deplacementNul();	
    } 
     
     public boolean deplacementtour(Deplacement deplacement){
        return deplacement.getDeplacementX() * deplacement.getDeplacementY() == 0 && !deplacement.deplacementNul();
    } 
      public boolean deplacementreine(Deplacement deplacement){
	return (Math.abs(deplacement.getDeplacementX()) - Math.abs(deplacement.getDeplacementY()) == 0 | deplacement.getDeplacementX() * deplacement.getDeplacementY() == 0) && !deplacement.deplacementNul();
    } 
}
