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
public interface MethodesEchiquier{
    public abstract void debuter();
    public abstract Case getCase(int ligne, int colonne);
    public abstract boolean cheminPossible(Deplacement deplacement);
    public abstract boolean captureParUnPionPossible(Deplacement deplacement);
}
