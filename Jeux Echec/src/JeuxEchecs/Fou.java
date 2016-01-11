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
public class Fou extends Piece{
    public Fou(String Couleur){
	super("Fou", Couleur);
    }
	
    public boolean deplacementValide(Deplacement deplacement){
		return Math.abs(deplacement.getDeplacementX()) - Math.abs(deplacement.getDeplacementY()) == 0 && !deplacement.deplacementNul();	
    }    
}
