/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package JeuxEchecs;

/**
 *
 * @authors Benayed, De preiter-Baise, Lottiaux
 */
public class Pion extends Piece{

    public Pion(String Couleur){
	super("Pion", Couleur);
    }

    public boolean deplacementValide(Deplacement deplacement){
	if(deplacement.getDeplacementX() == 0)
            if (this.getCouleur().equals("noir")){ 
                return deplacement.getDeplacementY() <= (deplacement.getDepart().getLigne() == 1 ? 2 : 1) && deplacement.getDeplacementY() > 0;
            }
            else{
                return deplacement.getDeplacementY() >= (deplacement.getDepart().getLigne() == 6 ? -2 : -1) && deplacement.getDeplacementY() < 0;
            }
            return false;				
	 
    }
}
