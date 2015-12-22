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
public class Position{
    private int ligne; 
    private int colonne; 

    public Position(int ligne, int colonne){
	this.ligne = ligne;
	this.colonne = colonne;
    }

    public void setLigne(int ligne){
	this.ligne = ligne;
    }
    
    public int getLigne(){
	return ligne;
    }
    	
    public void setColonne(int colonne){
	this.colonne = colonne;
    }

    public int getColonne(){
	return colonne;
    }

    public boolean equals(Position position){
        return position.getLigne() == this.getLigne() && position.getColonne() == this.getColonne();
    }    
}
