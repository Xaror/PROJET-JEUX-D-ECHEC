/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package JeuxEchecs;

import javax.swing.JFrame;
import java.awt.Dimension;
import javax.swing.JPanel;
import java.awt.Rectangle;
import javax.swing.BorderFactory;
import javax.swing.border.EtchedBorder;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import javax.swing.JLabel;
import javax.swing.JTextField;
import java.awt.Color;
import static javax.swing.JFrame.EXIT_ON_CLOSE;
/**
 *
 * @authors Benayed, De preiter-Baise, Lottiaux2
 */
public class JeuxEchecs extends JFrame{
    private Echiquier e; // echiquier
    private JLabel[][] tab; // tableau de JLabels
    private JPanel panelControle = new JPanel(); // panel du haut
    private JPanel panelGrille = new JPanel(); // panel du bas ( grille )
    GridLayout gridLayout1 = new GridLayout();
    private JButton boutonDebuter = new JButton();
    private JTextField champTexte = new JTextField();
    private JButton boutonReset = new JButton();
    private JPanel panelblanc = new JPanel();
    private JPanel panelnoir = new JPanel();
	
    public JeuxEchecs(){
	try{
            jbInit();
	}catch(Exception e){
            e.printStackTrace();
	}
    }
	
    private void jbInit() throws Exception{
        tab = new JLabel[8][8]; // création du tableau de JLabel
        e = new Echiquier(); // création de l'échiquier
        this.getContentPane().setLayout(null);
        this.setSize(new Dimension(784, 585));
        this.setTitle("Jeu d'Echecs");
        panelControle.setBounds(new Rectangle(5, 10, 550, 45));
        panelControle.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
        panelControle.setLayout(null);
        panelGrille.setBounds(new Rectangle(5, 65, 550, 465));
        panelGrille.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
        panelGrille.setLayout(gridLayout1);
        gridLayout1.setColumns(8);
        gridLayout1.setRows(8);
        this.getContentPane().add(panelnoir, null);
        this.getContentPane().add(panelblanc, null);
        this.getContentPane().add(panelGrille, null);
        panelControle.add(boutonReset, null);
        panelControle.add(champTexte, null);
        panelControle.add(boutonDebuter, null);
        this.getContentPane().add(panelControle, null);
        boutonDebuter.setBounds(new Rectangle(15, 10, 130, 25));
        boutonDebuter.setText("Nouveau");
        champTexte.setBounds(new Rectangle(160, 10, 215, 25));
        boutonReset.setText("Abandonner");
        boutonReset.setBounds(new Rectangle(390, 10, 130, 25));
        GestionnaireEvenement gest = new GestionnaireEvenement();
        boutonDebuter.addMouseListener(gest);
        boutonReset.addMouseListener(gest);
        panelblanc.setBounds(new Rectangle(570, 65, 75, 480));
        panelblanc.setBackground(new Color(255, 255, 255));
        panelblanc.setLayout(new FlowLayout());
        panelnoir.setBounds(new Rectangle(655, 65, 75, 475));
        panelnoir.setBackground(new Color(100, 100, 100));
        panelnoir.setLayout(new FlowLayout());
        int a = 1;
        for(int ligne = 0; ligne < 8; ligne++){
            a = a == 1 ? 0 : 1;
            for(int colonne = 0; colonne < 8; colonne++){
                tab[colonne][ligne] = new JLabel();
                tab[colonne][ligne].setOpaque(true);
                panelGrille.add(tab[colonne][ligne]);
                tab[colonne][ligne].setOpaque(true);
                tab[colonne][ligne].setHorizontalAlignment(SwingConstants.CENTER); 
                tab[colonne][ligne].addMouseListener(gest); 
                if((colonne + 1) % 2 == a){
                    tab[colonne][ligne].setBackground(new Color(255, 255, 255));
                }else{
                    tab[colonne][ligne].setBackground(new Color(100, 100, 100));
                }
            }
        }
    }

    private class GestionnaireEvenement extends MouseAdapter{
	Piece pieceTampon = null;
	ImageIcon iconeTampon;
	int ligneClic;
	int colonneClic;
	Position depart, arrivee;
	String couleurControle = "blanc";
	Position temp = null;
        
            public void mouseClicked(MouseEvent eve){
		// si on clique sur le bouton débuter
		if (eve.getSource() == boutonDebuter){
                    champTexte.setText("C'est le tour des blancs");
                    boutonDebuter.setEnabled(false);
                    e.debuter();
                    String dossierIcone = "Icone/";
                    char[] ordrePiece = { 'T', 'C', 'F', 'D', 'R', 'F', 'C', 'T' };
                    int increment = 1;
                    int ligne = 0;
                    char couleur = 'N';
                    Piece tempo = null;
                    e.debuter();
                    //Placement des images des pieces sur les cases
                    while(increment >= -1){
			for(int ctr = 0; ctr <= 7; ctr++){
                            tab[ctr][ligne].setIcon(new ImageIcon(dossierIcone + ordrePiece[ctr] + couleur + ".gif"));
                            switch(ordrePiece[ctr]){
				case 'T':
                                    tempo = new Tour(ligne < 5 ? "noir" : "blanc");
                                    break;
				case 'C':
                                    tempo = new Cavalier(ligne < 5 ? "noir" : "blanc");
                                    break;
				case 'F':
                                    tempo = new Fou(ligne < 5 ? "noir" : "blanc");
                                    break;
				case 'D':
                                    tempo = new Reine(ligne < 5 ? "noir" : "blanc");
                                    break;
				case 'R':
                                    tempo = new Roi(ligne < 5 ? "noir" : "blanc");
                                    break;
                            }
                            e.getCase(ctr, ligne).setPiece(tempo);
                            tab[ctr][ligne + increment].setIcon(new ImageIcon(dossierIcone + 'P' + couleur + ".gif"));
                            e.getCase(ctr, ligne + increment).setPiece(new Pion(ligne < 5 ? "noir" : "blanc"));
                        }
			couleur = 'B';
			increment -= 2;
			ligne = 7;
                    }
		}
		// si on clique sur le bouton reset
		else if(eve.getSource() == boutonReset){
                    RAZ();	
		}
                // si on a cliqué sur un Label
                else if(eve.getSource() instanceof JLabel){
                    for(int i = 0; i < 8; i++){
			for(int j = 0; j < 8; j++){
                            if(eve.getSource() == tab[j][i]){
				ligneClic = i;
				colonneClic = j;
                            }
                        }
                        //si on a cliqué sur une case non vide et que le tampon n'est pas null
                        if((e.getCase(colonneClic, ligneClic).getPiece() != null | pieceTampon != null) ){
                            //si le tampon est null
                            if(pieceTampon == null){
				if(e.getCase(colonneClic, ligneClic).getPiece().getCouleur().equals(couleurControle)){
                                    pieceTampon = e.getCase(colonneClic, ligneClic).getPiece();
                                    iconeTampon = (ImageIcon)tab[colonneClic][ligneClic].getIcon();
                                    temp = new Position(colonneClic,ligneClic);
                                    tab[colonneClic][ligneClic].setBorder(BorderFactory.createLineBorder(new Color(255, 0, 0),5));
                            }
                        }else{
                            Deplacement deplacement = new Deplacement(temp, new Position(colonneClic,ligneClic));
                            //je vérifie si le déplacement est valide, et si le chemin est possible, le pion de mange la piece
                            if((pieceTampon.deplacementValide(deplacement) && e.cheminPossible(deplacement)) | e.captureParUnPionPossible(deplacement)){
				//je créé un jLabel avec l'icône de la pièce mangée
				JLabel manger = new JLabel(tab[colonneClic][ligneClic].getIcon());
				manger.setHorizontalAlignment(SwingConstants.CENTER);
				//je l'ajoute au bon jPanel
				if (couleurControle.equals("blanc")){
                                    panelblanc.add(manger);
                                }else{		
                                    panelnoir.add(manger);
                                }				
				/* je vérifie si la pièce mangée est un roi, si oui le jeu est terminé et l'utilisateur peut choisir s'il veut continuer à jouer ou non*/
				if(e.getCase(colonneClic, ligneClic).getPiece() instanceof Roi){
                                    if(JOptionPane.showConfirmDialog(null, "Félicitation vous avez gagné ! Désirez-vous jouer de nouveau ?\n", "Mine !", JOptionPane.YES_NO_OPTION) == 0){
					RAZ();
					tab[temp.getColonne()][temp.getLigne()].setBorder(BorderFactory.createLineBorder(new Color(0, 0, 0),0));
                                    }else{
					System.exit(0);
                                    }
                                //si on dépose la pièce sur une case vide
                                }else{
                                    //on met le tampon sur la case vide et on vide le tampon ensuite
                                    e.getCase(temp.getColonne(), temp.getLigne()).setPiece(null);
                                    tab[temp.getColonne()][temp.getLigne()].setBorder(BorderFactory.createLineBorder(new Color(0, 0, 0),0));
                                    tab[colonneClic][ligneClic].setIcon(iconeTampon);
                                    e.getCase(colonneClic, ligneClic).setPiece(pieceTampon);
                                    tab[temp.getColonne()][temp.getLigne()].setIcon(null);
                                    pieceTampon = null;
                                    iconeTampon = null;
                                    temp = null;
                                    couleurControle = couleurControle.equals("blanc") ? "noir" : "blanc";
                                    champTexte.setText("C'est le tour des " + couleurControle);
				}
                            }else{
				tab[temp.getColonne()][temp.getLigne()].setBorder(BorderFactory.createLineBorder(new Color(0, 0, 0),0));
				pieceTampon = null;
				iconeTampon = null;
				temp = null;
                            }
			}
                    }
                }
            }
        }
    }

    //Remise de tout les attributs de la classe a 0
    public void RAZ(){
	for(int ligne = 0; ligne < 8; ligne++){ 
            for (int colonne = 0; colonne < 8; colonne++){
		tab[colonne][ligne].setIcon(null);
		e.getCase(colonne, ligne).setPiece(null);
            }
            champTexte.setText("");
            boutonDebuter.setEnabled(true);
            e.debuter();
            String couleurControle = "noir";
            panelblanc.removeAll();
            panelblanc.repaint();
            panelnoir.removeAll();
            panelnoir.repaint();
        }
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args){
        JeuxEchecs j = new JeuxEchecs();
	j.setVisible(true);
	j.setLocation(100, 130);
	j.setDefaultCloseOperation(EXIT_ON_CLOSE); // ferme le processus associé
    }
    
}
