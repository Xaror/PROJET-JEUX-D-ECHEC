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
public class Tour extends Piece{
    public Tour(String Couleur){
	super("Tour", Couleur);
    }

    public boolean deplacementValide(Deplacement deplacement){
        return deplacement.getDeplacementX() * deplacement.getDeplacementY() == 0 && !deplacement.deplacementNul();
    }    
}
