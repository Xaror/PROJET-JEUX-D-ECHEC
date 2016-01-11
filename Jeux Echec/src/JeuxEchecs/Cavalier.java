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
public class Cavalier extends Piece{
    public Cavalier(String Couleur){
	super("Cavalier", Couleur);
    }

    public boolean deplacementValide(Deplacement deplacement){
        return (Math.abs(deplacement.getDeplacementX() / deplacement.getDeplacementY())) == 2 | (Math.abs(deplacement.getDeplacementX() / deplacement.getDeplacementY())) == .5;
    }    
}
