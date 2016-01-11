package guichess;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Properties;
import java.util.ResourceBundle;
import java.util.concurrent.TimeUnit;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.image.PixelReader;
import javafx.scene.image.WritableImage;
import javafx.scene.input.MouseEvent;
import javafx.scene.paint.Color;
import javafx.stage.FileChooser;
import javafx.stage.FileChooser.ExtensionFilter;
import ucichess.UCIChess;
import ucichess.ChessBoard;

/**
 * FXML Controller class
 *
 * @author tondeur-h
 */
public class GUIController implements Initializable {
    @FXML
    private ImageView imgChessboard;
    @FXML
    private TextArea listTA;
    @FXML
    private Label lblTimeBlanc;
    @FXML
    private Label lblTimeNoir;
    @FXML
    private Button btnnouveau;
    @FXML
    private Button btnAbandon;
    @FXML
    private Label lblPlayer;
    @FXML
    private Button btnMoteur;


        Image tourB=null;
        Image tourN=null;

        Image fouB=null;
        Image fouN=null;

        Image cavB=null;
        Image cavN=null;

        Image dameB=null;
        Image dameN=null;

        Image roiB=null;
        Image roiN=null;


        Image pionB=null;
        Image pionN=null;

        Image imBoard=null;
        WritableImage plateau;
        WritableImage plateauInit;

        MyChessBoard mcb;

        UCIChess uci;

        int numClick;
        ArrayList<Click> posClick;

        boolean isEngineUP;

        int currentPlayer=Pieces.BLANCHE;
        String FEN=ChessBoard.STARTPOSITION;
        ArrayList<ChessBoard.Position> ar;

        final int BOARDSIZE=8;
        final int FULLBOARDSIZE=416;

        String movesStr="";
        boolean verrou=false; //jeton d'attente fin dessin

        boolean DEBUG=true;

        //gestion du temps
        long dureeBlack=0;
        long debutBlack=0;
        long finBlack=0;
        long dureeWhite=0;
        long finWhite=0;
        long debutWhite=0;


    /**
     * Initializes the controller class.
     * @param url
     * @param rb
     */
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        //initialiser la liste des clicks
        numClick=0;
        posClick=new ArrayList<>(2);
        posClick.add(new Click());posClick.add(new Click());
        //nom du joueur
        currentPlayer=Pieces.BLANCHE;
        lblPlayer.setText("les blancs jouent");
        //charger les images
        charge_pieces();
        //charger le moteur si possible
        btnMoteur.setText("MOTEUR : Aucun");
        String engine=read_Engine();
        //si aucun moteur alors seule solution le charger
        if (!isEngineUP){
            btnnouveau.setDisable(true);
            btnAbandon.setDisable(true);
        }
        else
        {
             startEngine(engine);
        }
    } //fin initialize


    /**
     * Lire le nom du moteur d'echec
     * @return
     */
    private String read_Engine(){
        Properties p=new Properties(); //ouvrir le fichier propriété
        try {
            p.loadFromXML(new FileInputStream("engine.xml"));
        } catch (FileNotFoundException ex) {
            //si fichier indisponible
           isEngineUP=false;
           return null;
        } catch (IOException ex) {
            //si fichier indisponible
           isEngineUP=false;
           return null;
        }
        isEngineUP=true;
        return p.getProperty("ENGINE");
    }


    /**
     * Sauvegarder moteur d'échec
     * @param path
     */
    private void save_Engine(String path){
        Properties p=new Properties(); //creer un properties
        p.setProperty("ENGINE", path); //affecter le chemin du moteur a la propriété ENGINE
        try {
            p.storeToXML(new FileOutputStream("engine.xml"), "HT ENGINE"); //sauvegarder
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
    }


    /**
     * lance le moteur d'échec...
     * @param path
     */
    private void startEngine(String path){
        uci=new UCIChess(path);
        uci.get_UciOk(DEBUG);
        btnMoteur.setText("Moteur : "+uci.getEngineName());
        init_board();
        affiche_Board();
        uci.send_uciNewGame();
    }


    /**
     * Méthode qui récupére le click sur l'image
     * du chessboard, transforme les coordonnées
     * relative de l'image en coordonnées du jeu d'échec
     * basé sur 0
     * @param event
     */
    @FXML
    private void chessboardClick(MouseEvent event) {
        //si le moteur n'est pas actif => sortir
        if (!isEngineUP) return;
        //si c'est le tour au noir => sortir
        if (currentPlayer==Pieces.NOIRE) return;
        /*
        La premiére case démarre au pixel Numero 30 et fait 45 pixels de haut 45 pixels de largeur
        on pratique donc l'opération (pointCourant-30)/45 => numero de la case
        */
        int casex=(int)((event.getX()-30)/45);
        int casey=(int)((event.getY()-30)/45);
        //Les case vont de 0 à 7 maximum (base 0)
        if (casex>7) casex=-1;
        if (casey>7) casey=-1;
        posClick.set(numClick, new Click(casex, casey)); //enregistre le click

        //verifier que le premier click est une case valide
        if (numClick==0) controle_click_un(posClick.get(0).getX(),posClick.get(0).getY());
        //verifier que le second click est une case valide
        if (numClick==1) controle_click_deux(posClick.get(0).getX(),posClick.get(0).getY(),posClick.get(1).getX(),posClick.get(1).getY());
        numClick++;

        if (numClick==2) {
            effectuer_deplacement_blanc(posClick.get(0).getX(),posClick.get(0).getY(),posClick.get(1).getX(),posClick.get(1).getY());
        }

    }



    private void position_du_roi(){

    }


    /**
     * Controler le premier click si piece est valide...
     * @param xd
     * @param yd
     */
    public void controle_click_un(int xd,int yd){
        //interdire la sélection d'une piéce noire
        if ((mcb.getPieces(xd, yd).getNom().compareTo(Pieces.PIONN)==0) ||
            (mcb.getPieces(xd, yd).getNom().compareTo(Pieces.TOURN)==0) ||
            (mcb.getPieces(xd, yd).getNom().compareTo(Pieces.CAVN)==0) ||
            (mcb.getPieces(xd, yd).getNom().compareTo(Pieces.FOUN)==0) ||
            (mcb.getPieces(xd, yd).getNom().compareTo(Pieces.DAMEN)==0) ||
            (mcb.getPieces(xd, yd).getNom().compareTo(Pieces.ROIN)==0)
            ){
        Alert alertNoir =new Alert(Alert.AlertType.ERROR, "Vous ne pouvez pas sélectionner une piéce noire!",ButtonType.CLOSE);
        alertNoir.showAndWait();
        numClick=0;
        affiche_Board(); //redessiner plateau
        return;
    }
        //interdire la sélection d'une case vide
        if((mcb.getPieces(xd, yd).getNom().compareTo(Pieces.VIDE)==0)){
        Alert alertVide =new Alert(Alert.AlertType.ERROR, "Vous ne pouvez pas sélectionner une case vide!",ButtonType.CLOSE);
        alertVide.showAndWait();
        numClick=0; //remettre à -1 car increment +1 ensuite => retour à 0
        affiche_Board(); //redessiner plateau
        return;
        }

        //tester si le roi est menacé ?
        if (ChessBoard.whiteKingIsThreat(4, 0)==true && mcb.getPieces(xd, yd).getNom().compareTo(Pieces.ROIB)!=0){
        Alert alertMenace =new Alert(Alert.AlertType.ERROR, "Mouvement illégal, votre roi est en échec!",ButtonType.CLOSE);
        alertMenace.showAndWait();
        numClick=-1; //remettre à -1 car increment +1 ensuite => retour à 0
        affiche_Board(); //redessiner plateau
        return;
        }

        //dessiner le cadre de sélection
        dessiner_cadre_selection(xd,yd);
    }


     /**
     * Controler le second click si piece est valide...
     * @param xd
     * @param yd
     */
    public void controle_click_deux(int xd,int yd,int xa,int ya){
        //ne pas choisir une case avec piéce blanche
        if ((mcb.getPieces(xa, ya).getNom().compareTo(Pieces.PIONB)==0) ||
            (mcb.getPieces(xa, ya).getNom().compareTo(Pieces.TOURB)==0) ||
            (mcb.getPieces(xa, ya).getNom().compareTo(Pieces.CAVB)==0) ||
            (mcb.getPieces(xa, ya).getNom().compareTo(Pieces.FOUB)==0) ||
            (mcb.getPieces(xa, ya).getNom().compareTo(Pieces.DAMEB)==0) ||
            (mcb.getPieces(xa, ya).getNom().compareTo(Pieces.ROIB)==0)
            ){
        Alert alertBlanc =new Alert(Alert.AlertType.ERROR, "Vous ne pouvez pas sélectionner une case occupée par une piéce de votre camps!",ButtonType.CLOSE);
        alertBlanc.showAndWait();
        numClick=0; //remettre à -1 car increment +1 ensuite => retour à 0
        //affiche_Board(); //redessiner le plateau
        return;
        }
    }


    /**
     * Mouvement du joueur noir
     */
    private void effectuer_deplacement_noir(){
        if (currentPlayer==Pieces.BLANCHE) return;
        debutBlack=System.currentTimeMillis();
        //si uci est pret
        if (uci.get_UciOk(DEBUG)){
            //noir reflechi
            uci.go_Think();
            //recuperer mouvement
            String blackMove=uci.get_BestMove(DEBUG);
            //appliquer le mouvement aux noirs
            uci.move_FromFEN(FEN, blackMove, DEBUG);
            //afficher chessBoard avec nouveau mouvement
            //ChessBoard.moveToCoord(blackMove);
            movesStr=movesStr+"Black: "+blackMove+"\n";
            listTA.setText(movesStr);

            //appliquer au Chessboard
            FEN=ChessBoard.moveFromFEN(FEN, blackMove);
            //appliquer au mode graphique
            mcb.setPieces(ChessBoard.getColTo(), 7-ChessBoard.getRowTo(),mcb.getPieces(ChessBoard.getColFrom(), 7-ChessBoard.getRowFrom()));
            mcb.delPieces(ChessBoard.getColFrom(), 7-ChessBoard.getRowFrom());
            affiche_Board();
            //DEBUG
            if (DEBUG) ChessBoard.show_chessboard();
        }

        currentPlayer=Pieces.BLANCHE;
         lblPlayer.setText("Les blancs jouent...");
         finBlack=System.currentTimeMillis();
         dureeBlack=dureeBlack+(finBlack-debutBlack);
         lblTimeNoir.setText(toMMSSCS(dureeBlack));
          debutWhite=System.currentTimeMillis();
    }


    /**
     * tester la demande de déplacement
     * @param xd
     * @param yd
     * @param xa
     * @param ya
     */
    private void effectuer_deplacement_blanc(int xd,int yd,int xa, int ya){

        boolean coupValide=false;
        //DEBUG
        if (DEBUG) {
        System.out.println("xd="+xd+" yd="+(7-yd));
        System.out.println("xa="+xa+" ya="+(7-ya));
        }

        //tester la validité du déplacement
        ar=ChessBoard.get_list_of_valid_moves(FEN, xd, (7-yd));
        for (int i=0;i<ar.size();i++){
            //DEBUG
        if (DEBUG) System.out.println("X="+((ChessBoard.Position)ar.get(i)).getCol()+" Y="+((ChessBoard.Position)ar.get(i)).getRow());

            if (xa==(((ChessBoard.Position)ar.get(i)).getCol()) && (7-ya)==(((ChessBoard.Position) ar.get(i)).getRow())){coupValide=true;}
        }

        if (coupValide){
            //recuperer pieces position depart
            numClick=0;
            mcb.setPieces(xa, ya,mcb.getPieces(xd, yd));
            mcb.delPieces(xd, yd);
            affiche_Board();

            //appliquer mouvement des blanc au moteur d'échec
            uci.get_UciOk(DEBUG);
            uci.move_FromFEN(FEN,ChessBoard.coordToMove(xd, 7-yd, xa, 7-ya, ""), DEBUG);

            //appliquer le mouvement au chessboard
            movesStr=movesStr+"White: "+ChessBoard.coordToMove(xd, 7-yd, xa, 7-ya, "")+"\n";
            listTA.setText(movesStr);

            FEN=ChessBoard.moveFromFEN(FEN, ChessBoard.coordToMove(xd, 7-yd, xa, 7-ya, ""));
            //DEBUG
            if (DEBUG) ChessBoard.show_chessboard();

//            //les noires jouent
            currentPlayer=Pieces.NOIRE;
             lblPlayer.setText("Les noirs jouent...");
             finWhite=System.currentTimeMillis();
             if (debutWhite==0) {debutWhite=finWhite;}
             dureeWhite=dureeWhite+(finWhite-debutWhite);
             System.out.println(dureeWhite);
             lblTimeBlanc.setText(toMMSSCS(dureeWhite));
            if (!verrou) effectuer_deplacement_noir();
        }
        else
        {
            //indiquer que le mouvement est invalide
            Alert alertInvalide=new Alert(Alert.AlertType.ERROR);
            alertInvalide.setTitle("Coup invalide!");
            alertInvalide.setContentText("Ce coup n'est pas valide!");
            alertInvalide.showAndWait();
            numClick=0;
            //affiche_Board(); //redessiner le plateau
        }
    }


    /**
     * click sur le bouton nouveau
     * @param event
     */
    @FXML
    private void btnNouveauClick(ActionEvent event) {
        uci.send_uciNewGame();
        uci.get_ReadyOk(DEBUG);
        init_board();
        affiche_Board();
    }


    /**
     * click sur le bouton abandonner
     * @param event
     */
    @FXML
    private void btnAbandonClick(ActionEvent event) {
        Alert alertAbondon=new Alert(Alert.AlertType.CONFIRMATION);
        alertAbondon.setContentText("Les blancs abandonnent...");
        alertAbondon.setTitle("Demande d'abandon...");
        if(alertAbondon.showAndWait().get()==ButtonType.OK){
            if (DEBUG) System.out.println("engine stopped");
            //uci.stop_Engine();
        }
    }


    /**
     * click sur le bouton moteur d'échec
     * @param event
     */
    @FXML
    private void btnMoteurClick(ActionEvent event) {
     FileChooser fileChooser = new FileChooser();
     fileChooser.setTitle("Ouvrir un moteur d'échec");
     fileChooser.getExtensionFilters().addAll(new ExtensionFilter("Chess Engine", "*.exe"));
     File selectedFile = fileChooser.showOpenDialog(null);
     if (selectedFile != null) {
        try{uci.stop_Engine();}catch (Exception e){}
        isEngineUP=true;
        btnnouveau.setDisable(false);
        btnAbandon.setDisable(false);
        save_Engine(selectedFile.getAbsolutePath());
        startEngine(selectedFile.getAbsolutePath());
        init_board();
        affiche_Board();
    }
    }


    /**
     * Ajouter 0 si digit sur 1 valeur
     * @param millis
     * @return
     */
    public String toMMSSCS(Long millis){
        long min=millis/60000;
        long sec=(millis%60000)/1000;
        long mil=((millis%60000)%1000);
        String strMin=Long.toString(min, 10);
        String strSec=Long.toString(sec, 10);
        String strMil=Long.toString(mil, 10);
        if (strMin.length()<2) strMin="0"+strMin;
        if (strSec.length()<2) strSec="0"+strSec;
        if (strMil.length()<2) strMil="0"+strMil;
        if (strMil.length()<3) strMil="0"+strMil;

        return strMin+":"+strSec+"."+strMil;
    }


/**
 * initialise l'échiquier de départ
 */
    private void init_board(){

        mcb=new MyChessBoard();
        //pieces blanches
        mcb.setPieces(0,7, new Pieces(Pieces.TOURB));
        mcb.setPieces(1,7, new Pieces(Pieces.CAVB));
        mcb.setPieces(2,7, new Pieces(Pieces.FOUB));
        mcb.setPieces(3,7, new Pieces(Pieces.DAMEB));
        mcb.setPieces(4,7, new Pieces(Pieces.ROIB));
        mcb.setPieces(5,7, new Pieces(Pieces.FOUB));
        mcb.setPieces(6,7, new Pieces(Pieces.CAVB));
        mcb.setPieces(7,7, new Pieces(Pieces.TOURB));
        for (int col=0;col<BOARDSIZE;col++){
            mcb.setPieces(col,6, new Pieces(Pieces.PIONB));
        }
        //pieces noires
        mcb.setPieces(0,0, new Pieces(Pieces.TOURN));
        mcb.setPieces(1,0, new Pieces(Pieces.CAVN));
        mcb.setPieces(2,0, new Pieces(Pieces.FOUN));
        mcb.setPieces(3,0, new Pieces(Pieces.DAMEN));
        mcb.setPieces(4,0, new Pieces(Pieces.ROIN));
        mcb.setPieces(5,0, new Pieces(Pieces.FOUN));
        mcb.setPieces(6,0, new Pieces(Pieces.CAVN));
        mcb.setPieces(7,0, new Pieces(Pieces.TOURN));
        for (int col=0;col<BOARDSIZE;col++){
            mcb.setPieces(col,1, new Pieces(Pieces.PIONN));
        }

    //remettre a zero compteur des joueurs
        lblTimeBlanc.setText("00:00:00");
        lblTimeNoir.setText("00:00:00");

     //remttre a zero nom du jour
        currentPlayer=Pieces.BLANCHE;
        lblPlayer.setText("Les blancs jouent");

        //initialise le chessboard de l'api UCI
        FEN=ChessBoard.STARTPOSITION;
        ChessBoard.assign_chessboard(FEN);
        //DEBUG
        if (DEBUG) ChessBoard.show_chessboard();
    }


/**
 * load pieces
 * Charge les images des piéces en mémoire...
 */
    private void charge_pieces(){
        imBoard=imgChessboard.getImage(); //conserver l'origine
        //la rendre Writable
        plateauInit=new WritableImage(imBoard.getPixelReader(), FULLBOARDSIZE, FULLBOARDSIZE);
        plateau=new WritableImage((int)plateauInit.getWidth(), (int)plateauInit.getHeight());

        //charger tour blanche
        try {
            tourB=new Image(new FileInputStream("./src/guichess/img/tourB.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
        //charger tour noire
        try {
            tourN=new Image(new FileInputStream("./src/guichess/img/tourN.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }

        //charger cav blanc
        try {
            cavB=new Image(new FileInputStream("./src/guichess/img/cavB.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
        //charger cav noir
        try {
            cavN=new Image(new FileInputStream("./src/guichess/img/cavN.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }

        //charger fou blanc
        try {
            fouB=new Image(new FileInputStream("./src/guichess/img/fouB.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
        //charger fou noir
        try {
            fouN=new Image(new FileInputStream("./src/guichess/img/fouN.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }

        //charger pion blanc
        try {
            pionB=new Image(new FileInputStream("./src/guichess/img/pionB.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
        //charger pion noir
        try {
            pionN=new Image(new FileInputStream("./src/guichess/img/pionN.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }


        //charger dame blanche
        try {
            dameB=new Image(new FileInputStream("./src/guichess/img/dameB.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
        //charger dame noire
        try {
            dameN=new Image(new FileInputStream("./src/guichess/img/dameN.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }


        //charger roi blanc
        try {
            roiB=new Image(new FileInputStream("./src/guichess/img/roiB.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
        //charger roi noir
        try {
            roiN=new Image(new FileInputStream("./src/guichess/img/roiN.png"));
        } catch (FileNotFoundException ex) {
            Logger.getLogger(GUIController.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /**
     * dessine un cadre de sélection sur le plateau
     * @param xd
     * @param yd
     */
    public void dessiner_cadre_selection(int xd,int yd){
        Color couleur=new Color(1.0, 0.0, 1.0, 1.0); //couleur rouge
        //lignes horizontales
        for (int x=0;x<44;x++){
                    plateau.getPixelWriter().setColor((31+x)+(44*(xd)), (33+0)+(43*(yd)), couleur);
                    plateau.getPixelWriter().setColor((31+x)+(44*(xd)), (33+42)+(43*(yd)), couleur);
        }
        //lignes verticales
        for (int y=0;y<43;y++){
                    plateau.getPixelWriter().setColor((31+0)+(44*(xd)), (33+y)+(43*(yd)), couleur);
                    plateau.getPixelWriter().setColor((31+43)+(44*(xd)), (33+y)+(43*(yd)), couleur);
        }
    }


/**
     * dessine un cadre de sélection sur le plateau
     * @param xd
     * @param yd
     * @param xa
     * @param ya
     */
    public void dessiner_cadre_selection2(int xd,int yd,int xa,int ya){
        Color couleur=new Color(1.0, 0.0, 0.0, 1.0); //couleur rouge
        //lignes horizontales
        for (int x=0;x<44;x++){
                    //case départ
                    plateau.getPixelWriter().setColor((31+x)+(44*(xd)), (33+0)+(43*(yd)), couleur);
                    plateau.getPixelWriter().setColor((31+x)+(44*(xd)), (33+42)+(43*(yd)), couleur);
                    //case arrivée
                    plateau.getPixelWriter().setColor((31+x)+(44*(xa)), (33+0)+(43*(ya)), couleur);
                    plateau.getPixelWriter().setColor((31+x)+(44*(xa)), (33+42)+(43*(ya)), couleur);
        }
        //lignes verticales
        for (int y=0;y<43;y++){
                    //case départ
                    plateau.getPixelWriter().setColor((31+0)+(44*(xd)), (33+y)+(43*(yd)), couleur);
                    plateau.getPixelWriter().setColor((31+43)+(44*(xd)), (33+y)+(43*(yd)), couleur);
                    //case arrivée
                    plateau.getPixelWriter().setColor((31+0)+(44*(xa)), (33+y)+(43*(ya)), couleur);
                    plateau.getPixelWriter().setColor((31+43)+(44*(xa)), (33+y)+(43*(ya)), couleur);

        }
    }

    /**
     * Afficher le chessBoard dans la fenêtre...
     */
    private void affiche_Board(){
        verrou=true;
        //recopier plateauInit dans plateau
        plateau.getPixelWriter().setPixels(0, 0, (int)plateauInit.getWidth(),(int) plateauInit.getHeight(), plateauInit.getPixelReader(), 0, 0);
        for (int casey=0;casey<BOARDSIZE;casey++){
               for (int casex=0;casex<BOARDSIZE;casex++){

            //lire la pieces de la case
            Pieces p=mcb.getPieces(casex, casey);

            //Tour blanche
            if (p.getNom().compareTo(Pieces.TOURB)==0){draw_pieces(casex, casey, tourB);}

            //Cavalier blanc
            if (p.getNom().compareTo(Pieces.CAVB)==0){draw_pieces(casex, casey, cavB);}

            //Fou blanc
            if (p.getNom().compareTo(Pieces.FOUB)==0){draw_pieces(casex, casey, fouB);}

            //dame blanche
            if (p.getNom().compareTo(Pieces.DAMEB)==0){draw_pieces(casex, casey, dameB);}

            //Roi blanc
            if (p.getNom().compareTo(Pieces.ROIB)==0){draw_pieces(casex, casey, roiB);}

            //Pions blanc
            if (p.getNom().compareTo(Pieces.PIONB)==0){draw_pieces(casex, casey, pionB);}

            //Tour noire
            if (p.getNom().compareTo(Pieces.TOURN)==0){draw_pieces(casex, casey, tourN);}

            //Cavalier noir
            if (p.getNom().compareTo(Pieces.CAVN)==0){draw_pieces(casex, casey, cavN);}

            //Fou noir
            if (p.getNom().compareTo(Pieces.FOUN)==0){draw_pieces(casex, casey, fouN);}

            //dame noire
            if (p.getNom().compareTo(Pieces.DAMEN)==0){draw_pieces(casex, casey, dameN);}

            //Roi noir
            if (p.getNom().compareTo(Pieces.ROIN)==0){draw_pieces(casex, casey, roiN);}

            //Pions noir
            if (p.getNom().compareTo(Pieces.PIONN)==0){draw_pieces(casex, casey, pionN);}

            }//fin casex
        }//fin casey

        //afficher le chessBoard
        imgChessboard.setImage(plateau);

        verrou=false;
    }


    /**
     * copier les pixels de l'image
     * @param casex
     * @param casey
     * @param piece
     */
    private void draw_pieces(int casex, int casey,Image piece){
        PixelReader pr=piece.getPixelReader(); //recuperer les pixels de la piéce
        for (int x=0;x<piece.getWidth();x++){
            for (int y=0;y<piece.getHeight();y++){
                Color couleur=pr.getColor(x, y); //recuperer couleur du point
                double b=couleur.getBlue();
                double g=couleur.getGreen();
                double r=couleur.getRed();
                if (b==0.0 && g==0.0 && r==1.0){
                    //si couleur rouge => do nothing
                }
                else
                {
                    //ecrire le pixel sur le chessBoard
                    plateau.getPixelWriter().setColor((32+x)+(44*(casex)), (35+y)+(43*(casey)), couleur);
                }
            }
        }
    }


}