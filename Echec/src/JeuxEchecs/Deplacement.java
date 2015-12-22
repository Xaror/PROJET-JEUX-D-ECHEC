/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package jeuxechecs;

/**
 *
 * @authors Benayed, De preiter-Baise, Lottiaux
 */
public class Deplacement{
    private double deplacementX;
    private double deplacementY;
    private Position arrivee;
    private Position depart;
	
    public Deplacement(Position depart, Position arrivee){
	this.arrivee = arrivee;
	this.depart = depart;
	this.deplacementX = arrivee.getColonne() - depart.getColonne();
	this.deplacementY = arrivee.getLigne() - depart.getLigne();
    }

    public double getDeplacementX(){
	return deplacementX;
    }

    public double getDeplacementY(){
	return deplacementY;
    }
	
    public Position getArrivee(){
	return arrivee;
    }

    public Position getDepart(){
	return depart;
    }
	
    public boolean deplacementNul(){
	return deplacementX == 0 && deplacementY == 0;
    }  
}
