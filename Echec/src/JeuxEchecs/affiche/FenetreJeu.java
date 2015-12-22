/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package JeuxEchecs.affiche;

/**
 *
 * @author Bloody
 */

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


public class FenetreJeu extends JFrame {
	private static final long serialVersionUID = 42L; //serialVersionUID permet d'affecter un numéro de version à la classe
	//construction tableau echequier
	//private Echiquier e; // echiquier
	private JLabel[][] tab; // tableau de JLabels:champs non modifiable
	private JPanel panelControle = new JPanel(); // panel du haut
	private JPanel panelGrille = new JPanel(); // panel du bas ( grille )
	GridLayout gridLayout1 = new GridLayout();

	private JButton boutonDebuter = new JButton();
	private JTextField champTexte = new JTextField();
	private JButton boutonfin = new JButton();
	private JPanel panelblanc = new JPanel();
	private JPanel panelnoir = new JPanel();
	private JPanel panellens = new JPanel();
	private JPanel joueurblanc = new JPanel();
	private JPanel joueurnoir = new JPanel();
	JLabel banniere = new JLabel( new ImageIcon( "banniere.jpg"));
	JLabel jblanc = new JLabel( new ImageIcon( "jblanc.jpg"));
	JLabel jnoir = new JLabel( new ImageIcon( "jnoir.jpg"));
	
	 //Constructeur
	public FenetreJeu()
	{
		try {
			initialisationPanel(); //appelle methode initialisationPanel
		} catch (Exception e) {
			e.printStackTrace(); //affiche l'exception au moment de son appel
	    }
	}

	
	 // initialise la surface de jeu. Cre tout les elements et initialise leur position, leur couleur.. etc
	 
	private void initialisationPanel() throws Exception {

		tab = new JLabel[8][8]; // création du tableau de JLabel
		//e = new Echiquier(); // création de l'échiquier

		this.getContentPane().setLayout(null); // permet de centrer le cadre du haut
		this.setSize(new Dimension(900, 700)); //dimension fenetre 
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE); //fermer la fenetre
		this.setTitle("ECHEC"); //titre de la fenetre
		panelControle.setBounds(new Rectangle(160, 10, 550, 45)); //dimension du bloc du haut
		panelControle.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED)); //bordure cadre du haut
		panelControle.setLayout(null); //permet de voir le champ en haut
		panelGrille.setBounds(new Rectangle(160, 65, 550, 465)); //dimension de l'échiquier
		panelGrille.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
		panelGrille.setLayout(gridLayout1); // insert les colonnes et les lignes pour les cases
		gridLayout1.setColumns(8);
		gridLayout1.setRows(8);
		this.getContentPane().add(panelnoir, null); //visualiser le recuperateur de piece noir
		this.getContentPane().add(panelblanc, null);//visualiser le recuperateur de piece blanche
		this.getContentPane().add(panellens, null);//visualiser la banniere de lens
		this.getContentPane().add(joueurblanc, null);//visualiser le joueur blanc
		this.getContentPane().add(joueurnoir, null);//visualiser le joueur noir
		this.getContentPane().add(panelGrille, null); //visualiser lechiquier
		panelControle.add(boutonfin, null);
		panelControle.add(champTexte, null);
		panelControle.add(boutonDebuter, null);
		this.getContentPane().add(panelControle, null); //visualiser le champ text en haut
		boutonDebuter.setBounds(new Rectangle(15, 10, 130, 25));
		boutonDebuter.setText("Commencer");
		champTexte.setBounds(new Rectangle(160, 10, 215, 25));

		// les écouteurs
		boutonfin.setText("Terminé");
		boutonfin.setBounds(new Rectangle(390, 10, 130, 25));
		//GestionnaireEvenement gest = new GestionnaireEvenement();
		//boutonDebuter.addMouseListener(gest);
		//boutonfin.addMouseListener(gest);
		
		//crŽation des labels
		panelblanc.setBounds(new Rectangle(30, 65, 90, 465)); // dimension recuperateur de piece blanche
		panelblanc.setBackground(new Color(255, 0, 0));
		panelblanc.setLayout(new FlowLayout());
		panelblanc.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
		panelnoir.setBounds(new Rectangle(750, 65, 90, 465)); // dimension recuperateur de piece noire
		panelnoir.setBackground(new Color(0, 0, 0));
		panelnoir.setLayout(new FlowLayout());
		panelnoir.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
		panellens.setBounds(new Rectangle(160, 550, 550, 100)); 
		panellens.setLayout(new FlowLayout());
		panellens.add(banniere);
		joueurblanc.setBounds(new Rectangle(20, 1, 115, 60)); // dimension joueur blanc
		joueurblanc.setLayout(new FlowLayout());
		joueurblanc.add(jblanc);
		joueurnoir.setBounds(new Rectangle(735, 1, 115, 60)); // dimension joueur noir
		joueurnoir.setLayout(new FlowLayout());
		joueurnoir.add(jnoir);

		
		
		//J'attribue la couleur aux JLabels
		int a = 1;
		for (int ligne = 0; ligne < 8; ligne++) {
			a = a == 1 ? 0 : 1;
			for (int colonne = 0; colonne < 8; colonne++) {
				//tab[colonne][ligne] = new Case();
				panelGrille.add(tab[colonne][ligne]); // ajouter au Panel
				tab[colonne][ligne].setOpaque(true);
				tab[colonne][ligne].setHorizontalAlignment(SwingConstants.CENTER); //centre les pieces dans les cases
				//tab[colonne][ligne].addMouseListener(gest); 
				if ((colonne + 1) % 2 == a)
					tab[colonne][ligne].setBackground(new Color(255, 255, 255)); //couleur des cases : blanc
					else
					tab[colonne][ligne].setBackground(new Color(153, 153, 153)); //couleur des cases : noir
					}
			}
		}
		

	
	private void setCouleur() {
		/*if ( couppossible )
			setBackground(new Color(0, 240, 0)); //couleur d'un coup possible : vert
	*/	
            if (true)
			setBackground(new Color(255, 255, 255)); //couleur des cases : blanc
		else
			setBackground(new Color(153, 153, 153)); //couleur des cases : noir
	/*
	for (int ligne = 0; ligne < 8; ligne++) {
		   for (int colonne = 0; colonne < 8; colonne++) {
		      case = new Position(colonne,ligne);
		      Deplacement depalacement=new Deplacement(temp,case);
		      tab[colonne][ligne].setCoupPossible(deplacement.isValide());
		   }
		}
	for (int ligne = 0; ligne < 8; ligne++) {
		   for (int colonne = 0; colonne < 8; colonne++) {
		      tab[colonne][ligne].setCoupPossible(false);
		   }
		}
                */
        }
	/*public void setCoupPossible(boolean couppossible) {
	    this.couppossible=couppossible;
	    setCouleur();
	}
	public void setCouleur(boolean blanc) {
	    this.blanc=blanc;
	    setCouleur();
	}
	// classe privee pour la gestion des evenement de la souris.
	 
	private class GestionnaireEvenement extends MouseAdapter {

		Piece pieceTampon = null;
		ImageIcon iconeTampon;
		int ligneClic;
		int colonneClic;
		String couleurControle = "blanc";
		Position temp = null;



	
		public void mouseClicked(MouseEvent eve) {
			// si on clique sur le bouton débuter
			if (eve.getSource() == boutonDebuter) {
				//initialise le champ texte, apelle la methode debuter, et initialise toute les variables 
				champTexte.setText("blanc a vous de jouer");
                champTexte.setEnabled(false);
				boutonDebuter.setEnabled(false);
				e.debuter(); // appel
				String dossierIcone = "Icone/";
				char[] ordrePiece = { 'T', 'C', 'F', 'D', 'R', 'F', 'C', 'T' };
				int increment = 1;
				int ligne = 0;
				char couleur = 'N';
				Piece tempo = null;
				e.debuter(); // appel

				// Je place les icones des pieces sur leur cases respectives
				while (increment >= -1) {
					for (int ctr = 0; ctr <= 7; ctr++) {
						tab[ctr][ligne].setIcon(new ImageIcon(dossierIcone + ordrePiece[ctr] + couleur + ".gif"));
						switch(ordrePiece[ctr])
						{
						case 'T':
							tempo = new Tour(ligne < 5 ? "noir " : "blanc");
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
			// si on clique sur le bouton recommencer
			else if (eve.getSource() == boutonfin) {
				//on appel la methode RAZ
				RAZ();

				
			}

			else if (eve.getSource() instanceof JLabel) // donc on a cliqué sur un Label
			{
				for (int i = 0; i < 8; i++)
					//on determine sur quel Jlabel on a clique
					for (int j = 0; j < 8; j++) 
						if (eve.getSource() == tab[j][i]) {
							ligneClic = i;
							colonneClic = j;
						}
					//si on a clique sur une case non vide et que le tampon n'est pas null
					if((e.getCase(colonneClic, ligneClic).getPiece() != null | pieceTampon != null) )
					{
						//si le tampon est null
						if(pieceTampon == null )
						{
							//si c'est au tour de la couleur de controle a jouer
							if(e.getCase(colonneClic, ligneClic).getPiece().getCouleur().equals(couleurControle)){
								//J'initialise la piece tampon a la piece sur laquelle on a clique
								pieceTampon = e.getCase(colonneClic, ligneClic).getPiece();
								iconeTampon = (ImageIcon)tab[colonneClic][ligneClic].getIcon();
								temp = new Position(colonneClic,ligneClic);
								tab[colonneClic][ligneClic].setBorder(BorderFactory.createLineBorder(new Color(255,0 ,0),5));
									
							}
							
						}
						else
						{
							//je cree un deplacement
							Deplacement deplacement = new Deplacement(temp, new Position(colonneClic,ligneClic));
							//je verifie si le deplacement est valide, si le chemin est possible et si il est possible, pour un pion de manger la piece
							if ((pieceTampon.estValide(deplacement) && e.cheminPossible(deplacement)) | e.captureParUnPionPossible(deplacement))
							{
								//je cree un jLabel avec l'icone de la piece manger
								JLabel manger = new JLabel(tab[colonneClic][ligneClic].getIcon());
								manger.setHorizontalAlignment(SwingConstants.CENTER);
								
								//je l'ajoute au bon jPanel
								if (couleurControle.equals("blanc"))
									panelblanc.add(manger);
								else		
									panelnoir.add(manger);
								
								// je verifie si la piece manger est un roi, si oui le jeu est termine et l'utilisateur peut choisir si il veut continuer a jouer ou non
								if(e.getCase(colonneClic, ligneClic).getPiece() instanceof Roi)
								{
									if(JOptionPane.showConfirmDialog(null, "Felicitation vous avez gagne ! Desirez-vous jouer de nouveau ?\n", "Victoire !!!", JOptionPane.YES_NO_OPTION) == 0){
										RAZ();
										tab[temp.getColonne()][temp.getLigne()].setBorder(BorderFactory.createLineBorder(new Color(0, 0, 0),0)); // j'enleve le cadre rouge de la piece selectionne
									}

									else
										System.exit(0);

								}
								else//si on depose la piece sur une case vide
								{
									//on met le tampon sur la case vide et on vide le tampon apres
									e.getCase(temp.getColonne(), temp.getLigne()).setPiece(null);
									tab[temp.getColonne()][temp.getLigne()].setBorder(BorderFactory.createLineBorder(new Color(0, 0, 0),0)); // j'enleve le cadre rouge de la piece selectionne
									tab[colonneClic][ligneClic].setIcon(iconeTampon);
									e.getCase(colonneClic, ligneClic).setPiece(pieceTampon); // pour pouvoir le rebouger plusieurs fois dans une partie
									tab[temp.getColonne()][temp.getLigne()].setIcon(null); // permet de faire bouger la piece selectionne en supprimant la piece bouger de sa position initiale 

	
									pieceTampon = null;
									iconeTampon = null;
									temp = null;
	
									couleurControle = couleurControle.equals("blanc") ? "noir" : "blanc";
									champTexte.setText(couleurControle + " , a vous de jouer");
                                    		
								}
							}
							else
							{
								tab[temp.getColonne()][temp.getLigne()].setBorder(BorderFactory.createLineBorder(new Color(255, 0, 0),0));
								pieceTampon = null;
								iconeTampon = null;
								temp = null;

							}
						
						}

					}
					
				}

		}
	}

	//Je remet tout les attributs de la classe a 0
	public void RAZ()
	{
		for (int ligne = 0; ligne < 8; ligne++) 
			for (int colonne = 0; colonne < 8; colonne++) {
				tab[colonne][ligne].setIcon(null);
				e.getCase(colonne, ligne).setPiece(null);
				
			}
		champTexte.setText("");
		boutonDebuter.setEnabled(true);
		e.debuter();

		panelblanc.removeAll();
		panelblanc.repaint();
		panelnoir.removeAll();
		panelnoir.repaint();

	}
*/
	// main pour pouvoir exécuter l'interface graphique
	public static void main(String[] args) {
		FenetreJeu j = new FenetreJeu();
		j.setVisible(true);
		j.setLocationRelativeTo(null);
		j.setDefaultCloseOperation(EXIT_ON_CLOSE); // ferme le processus associé
	}
}