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
public class Position{
    private int ligne; 
    private int colonne; 

    public Position(int colonne, int ligne){
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
	return position.getColonne() == this.getColonne() && position.getLigne() == this.getLigne();
    }   
}