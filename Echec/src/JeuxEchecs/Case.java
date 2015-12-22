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
public class Case{
    private Piece piece;
	
    public Case(){
		
    }
	
    public Case(Piece piece){
	this.piece = piece;
    }
	
    public void setPiece(Piece piece){
	this.piece = piece;
    }
	
    public Piece getPiece(){
	return piece;
    }
		
    public boolean caseOccupee(String couleur){
	if (piece == null){
            return false;
        }
        else{
            return (piece.getCouleur().equals(couleur));
        }
    }  
    
    public boolean caseOccupee(){
	return (piece != null);	
    }
}
