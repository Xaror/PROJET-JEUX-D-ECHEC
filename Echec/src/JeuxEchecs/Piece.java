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
public abstract class Piece{
    private String nom;
    private String couleur;

    public Piece(String nom, String couleur){
	setNom(nom);
	setCouleur(couleur);
    }
    
    public void setNom(String nom){
	this.nom = nom;
    }
    public String getNom(){
	return nom;
    }
    
    public void setCouleur(String couleur){
	if ((couleur == "noir") || (couleur == "blanc"))
	this.couleur = couleur;
    }
    
    public String getCouleur(){
	return couleur;
    }
    
    public abstract boolean deplacementValide(Deplacement deplacement);
}
