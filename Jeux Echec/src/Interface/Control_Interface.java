/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Interface;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Optional;
import java.util.Properties;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Platform;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Alert.AlertType;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Label;
import javafx.scene.control.ListView;
import javafx.scene.control.TextArea;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.image.WritableImage;
import javafx.scene.input.ClipboardContent;
import javafx.scene.input.DragEvent;
import javafx.scene.input.Dragboard;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.TransferMode;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.Pane;
import javafx.scene.layout.StackPane;
import javafx.scene.text.Text;
import javafx.stage.Stage;
import static java.lang.Thread.sleep;
import javafx.scene.Node;
import static jdk.nashorn.internal.runtime.Context.DEBUG;
import ucichess.UCIChess;
import ucichess.ChessBoard;
/**
 *
 * @author Bloody
 */
public class Control_Interface implements Initializable {
    
    @FXML
    private Button closeButton;
    @FXML
    private Button StartGame;
    @FXML
    private Button Abandon;
    @FXML
    private Button btnMoteur;
    
    @FXML
    private Label tmpW;
    @FXML
    private Label tmpB;
     @FXML
    private Label Tour;
    @FXML
    private ListView listCpW;
    @FXML
    private ListView listCpB;
    @FXML
    private GridPane grille;
    
    boolean tour_blanc = true;
    boolean isEngineUP;
    private ArrayList<Piece> Piece;
    /**/
    private Chronometre ChronoW ;
    private Chronometre ChronoB;
    private boolean rock_b_possible = true;
    private boolean rock_n_possible = true;
    
    //UCIChess uci = new UCIChess("./src/stockfish-6-win/Windows/stockfish-6-64.exe");

    
    String FEN=ChessBoard.STARTPOSITION;
    ArrayList<ChessBoard.Position> ar;
    
    
    String moves=null; //keep moves in a String

    final boolean traceMode=false;
    final long timeThinking=100; //time thinking in miliseconds
    String fenPos="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
    
    
    
       //charger tour 
            Piece TB1 = new Piece("Tour",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TB.gif")))),"blanc");     
            Piece TN1 = new Piece("Tour",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TN.gif")))),"noire");
            Piece TB2 = new Piece("Tour",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TB.gif")))),"blanc");
            Piece TN2 = new Piece("Tour",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TN.gif")))),"noire");
        
        //charger cav     
            Piece CB1 = new Piece("Cavalier",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CB.gif")))),"blanc");
            Piece CN1 = new Piece("Cavalier",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CN.gif")))),"noire");
            Piece CB2 = new Piece("Cavalier",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CB.gif")))),"blanc");
            Piece CN2 = new Piece("Cavalier",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CN.gif")))),"noire");
            
        //charger fou   
            Piece FB1 = new Piece("Fou",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FB.gif")))),"blanc");
            Piece FN1 = new Piece("Fou",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FN.gif")))),"noire");
            Piece FB2 = new Piece("Fou",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FB.gif")))),"blanc");
            Piece FN2 = new Piece("Fou",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FN.gif")))),"noire");
            
         //charger dame     
            Piece DB = new Piece("Reine",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/DB.gif")))),"blanc");
            Piece DN = new Piece("Reine",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/DN.gif")))),"noire");
            
        //charger roi    
            Piece RB = new Piece("Roi",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/RB.gif")))),"blanc");
            Piece RN = new Piece("Roi",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/RN.gif")))),"noire");
           
          
        //charger pion     
            Piece PB1 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN1 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            Piece PB2 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN2 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            Piece PB3 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN3 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            Piece PB4 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN4 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            Piece PB5 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN5 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            Piece PB6 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN6 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            Piece PB7 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN7 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            Piece PB8 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")))),"blanc");
            Piece PN8 = new Piece("Pion",( new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")))),"noire");
            
           
        
            
    
        private void charge_pieces(){           
            //+grille.setGridLinesVisible(true);
            final int appsPerRow = 8;
            
            for (int i = 0; i < 63; i++) {
                createApp(grille, i, appsPerRow, false);
            }
            
                Pane app = createApp(grille, 0, appsPerRow, false);
                 app.getChildren().add(TN1.getimg());
                 TN1.setid(app.getChildren().toString());
                 Piece.add(TN1);
                 
                 app = createApp(grille, 1, appsPerRow, false);
                 app.getChildren().add(CN1.getimg());
                 CN1.setid(app.getChildren().toString());
                 Piece.add(CN1);
                 
                 app = createApp(grille, 2, appsPerRow, false);
                 app.getChildren().add(FN1.getimg());
                 FN1.setid(app.getChildren().toString());
                 Piece.add(FN1);
                
                 app = createApp(grille, 3, appsPerRow, false);
                 app.getChildren().add(RN.getimg());
                 RN.setid(app.getChildren().toString());
                 Piece.add(RN);
                 
                 app = createApp(grille, 4, appsPerRow, false);
                 app.getChildren().add(DN.getimg());
                 DN.setid(app.getChildren().toString());
                 Piece.add(DN);
                 
                 app = createApp(grille, 5, appsPerRow, false);
                 app.getChildren().add(FN2.getimg());
                 FN2.setid(app.getChildren().toString());
                 Piece.add(FN2);
                 
                 app = createApp(grille, 6, appsPerRow, false);
                 app.getChildren().add(CN2.getimg());
                 CN2.setid(app.getChildren().toString());
                 Piece.add(CN2);
                 
                 app = createApp(grille, 7, appsPerRow, false);
                 app.getChildren().add(TN2.getimg());
                 TN2.setid(app.getChildren().toString());
                 Piece.add(TN2);
                 
                 app = createApp(grille, 8, appsPerRow, false);
                 app.getChildren().add(PN1.getimg());
                 PN1.setid(app.getChildren().toString());
                 Piece.add(PN1);
                 
                 app = createApp(grille, 9, appsPerRow, false);
                 app.getChildren().add(PN2.getimg());
                 PN2.setid(app.getChildren().toString());
                 Piece.add(PN2);
                 
                 app = createApp(grille, 10, appsPerRow, false);
                 app.getChildren().add(PN3.getimg());
                 PN3.setid(app.getChildren().toString());
                 Piece.add(PN3);
                 
                 app = createApp(grille, 11, appsPerRow, false);
                 app.getChildren().add(PN4.getimg());
                 PN4.setid(app.getChildren().toString());
                 Piece.add(PN4);
                 
                 app = createApp(grille, 12, appsPerRow, false);
                 app.getChildren().add(PN5.getimg());
                 PN5.setid(app.getChildren().toString());
                 Piece.add(PN5);
                 
                 app = createApp(grille, 13, appsPerRow, false);
                 app.getChildren().add(PN6.getimg());
                 PN6.setid(app.getChildren().toString());
                 Piece.add(PN6);
                 
                 app = createApp(grille, 14, appsPerRow, false);
                 app.getChildren().add(PN7.getimg());
                 PN7.setid(app.getChildren().toString());
                 Piece.add(PN7);
                 
                 app = createApp(grille, 15, appsPerRow, false);
                 app.getChildren().add(PN8.getimg());
                 PN8.setid(app.getChildren().toString());
                 Piece.add(PN8);
                 
                 
                 
                 app = createApp(grille, 56, appsPerRow, false);
                 app.getChildren().add(TB1.getimg());
                 TB1.setid(app.getChildren().toString());
                 Piece.add(TB1);
                 
                 app = createApp(grille, 57, appsPerRow, false);
                 app.getChildren().add(CB1.getimg());
                 CB1.setid(app.getChildren().toString());
                 Piece.add(CB1);
                 
                 app = createApp(grille, 58, appsPerRow, false);
                 app.getChildren().add(FB1.getimg());
                 FB1.setid(app.getChildren().toString());
                 Piece.add(FB1);
                 
                 app = createApp(grille, 59, appsPerRow, false);
                 app.getChildren().add(RB.getimg());
                 RB.setid(app.getChildren().toString());
                 Piece.add(RB);
                 
                 app = createApp(grille, 60, appsPerRow, false);
                 app.getChildren().add(DB.getimg());
                 DB.setid(app.getChildren().toString());
                 Piece.add(DB);
                                  
                 app = createApp(grille,61, appsPerRow, false);
                 app.getChildren().add(FB2.getimg());
                 FB2.setid(app.getChildren().toString());
                 Piece.add(FB2);
                 
                 app = createApp(grille, 62, appsPerRow, false);
                 app.getChildren().add(CB2.getimg());
                 CB2.setid(app.getChildren().toString());
                 Piece.add(CB2);
                 
                 app = createApp(grille, 63, appsPerRow, false);
                 app.getChildren().add(TB2.getimg());
                 TB2.setid(app.getChildren().toString());
                 Piece.add(TB2);                 
                 
                 app = createApp(grille, 55, appsPerRow, false);
                 app.getChildren().add(PB1.getimg());
                 PB1.setid(app.getChildren().toString());
                 Piece.add(PB1);
                 
                 app = createApp(grille, 54, appsPerRow, false);
                 app.getChildren().add(PB2.getimg());
                 PB2.setid(app.getChildren().toString());
                 Piece.add(PB2);
                 
                 app = createApp(grille, 53, appsPerRow, false);
                 app.getChildren().add(PB3.getimg());
                 PB3.setid(app.getChildren().toString());
                 Piece.add(PB3);
                 
                 app = createApp(grille, 52, appsPerRow, false);
                 app.getChildren().add(PB4.getimg());
                 PB4.setid(app.getChildren().toString());
                 Piece.add(PB4);
                 
                 app = createApp(grille, 51, appsPerRow, false);
                 app.getChildren().add(PB5.getimg());
                 PB5.setid(app.getChildren().toString());
                 Piece.add(PB5);
                 
                 
                 app = createApp(grille, 50, appsPerRow, false);
                 app.getChildren().add(PB6.getimg());
                 PB6.setid(app.getChildren().toString());
                 Piece.add(PB6);
                 
                 
                 app = createApp(grille, 49, appsPerRow, false);
                 app.getChildren().add(PB7.getimg());
                 PB7.setid(app.getChildren().toString());
                 Piece.add(PB7);
                 
                 app = createApp(grille, 48, appsPerRow, false);
                 app.getChildren().add(PB8.getimg());
                 PB8.setid(app.getChildren().toString());
                 Piece.add(PB8);
                 
                  
        }
       
        
        
        
        private Pane createApp(final GridPane root, final int appNumber,final int appsPerRow, boolean filler) {
            final Pane app = new StackPane();
            

            final int x = appNumber % appsPerRow;
            final int y = appNumber / appsPerRow;
           // System.out.println("x " + x);
           // System.out.println("y " + y);
            root.add(app, x, y);
            app.setMinWidth(55);
            app.setMinHeight(55);
            if (!filler) {
              app.setOnDragDetected(new EventHandler<MouseEvent>() {
                @Override
                public void handle(MouseEvent event) {
                  Dragboard db = app.startDragAndDrop(TransferMode.MOVE);
                  ClipboardContent cc = new ClipboardContent();
                  cc.putString(String.valueOf(appNumber));
                  db.setContent(cc);

                  // JavaFX 8 only:
                  Image img = app.snapshot(null, null);
                  db.setDragView(img, 0, 0);

                  event.consume();
                }
              });
              app.setOnDragOver(new EventHandler<DragEvent>() {
        @Override
        public void handle(DragEvent event) {
                Dragboard db = event.getDragboard();
                boolean accept = false;
                if (db.hasString()) {
                  String data = db.getString();
                  try {
                    int draggedAppNumber = Integer.parseInt(data);
                    if (draggedAppNumber != appNumber
                        && event.getGestureSource() instanceof Pane) {
                      accept = true;
                    }
                  } catch (NumberFormatException exc) {
                    accept = false;
                  }
                }
                if (accept) {
                  event.acceptTransferModes(TransferMode.MOVE);
                }
              }
            });
            app.setOnDragDropped(new EventHandler<DragEvent>() {

                
        
        
       public boolean cible_roi(Piece j,Pane cible,String couleur){
           if(j.getnom().equals("Roi")){
                                cible.getChildren().clear();                                                
                                Alert alert = new Alert(AlertType.CONFIRMATION);
                                alert.setTitle("Echec et Mate");
                                alert.setHeaderText("Les "+ couleur +"s on gagné !!!");
                                alert.setContentText("Voulez vous recommencer une partie ou quiiter le jeu?");
                                Optional<ButtonType> result = alert.showAndWait();
                                if (result.get() == ButtonType.OK){
                                    
                                    restart();
                                    return  false;
                                    
                                } else {
                                    Platform.exit();
                                }              
                            }else cible.getChildren().clear();
           return  false;
       }
        public boolean test_move_valide(String couleur_inverse,String couleur,Pane cible,Piece i,int draggedX,int draggedY,int droppedX,int droppedY, boolean rock){
            boolean deplacement_ok = true;
            boolean ennemi = false;
                
            for(Piece j:Piece){
                
                   

                // test si la case cible contient une piece 
               // System.out.println(" passe " + cible.getChildren().toString() + " " +j.getid());
                if(cible.getChildren().toString().equals(j.getid())){
                    System.out.println(" passe1 " + cible.getChildren().toString());
                    // test si la case contient une piece ennemi 
                    if(j.gettype().equals(couleur_inverse)){
                        //ennemi detecte
                        System.out.println(ennemi);
                        ennemi = true;
                        System.out.println(ennemi);
                        //test si la piece source est un pion
                        if(i.getnom().equals("Pion") ){
                            System.out.println(" passe2 ");
                            // test si le deplaccement n'est en diagonal pas de prise de piece mouvement impossible
                            if((draggedX-droppedX==1 || draggedX-droppedX== -1) && i.deplacementValide(draggedX,draggedY, droppedX, droppedY, ennemi) ){
                                System.out.println(" passe2.5 ");
                                cible.getChildren().clear();
                                return true; 
                                
                            }else return false;
                            
                        }else{
                                //si ce n'est pas un pion mais il y a une piece ennemi
                                
                                
                                
                                //test deplacement valide 
                                System.out.println(" passe3 ");
                                if(!i.deplacementValide(draggedX,draggedY, droppedX, droppedY, ennemi)){
                                    System.out.println(i.deplacementValide(draggedX,draggedY, droppedX, droppedY, ennemi));
                                                return false;
                                }
                            }
                            cible_roi(j,cible,couleur);
                            
                            
                            
                        }         
                    
                     
                    
                    if(j.gettype().equals(couleur)){
                        if((j.getnom().equals("Tour") && i.getnom().equals("Roi")) || (i.getnom().equals("Tour") && j.getnom().equals("Roi")) ){
                            if(rock == true)
                        {    

                                                        rock = false;

                                                    }else{
                                                        return false;
                                                    }
                        }else return false;
                    }
                }else{
                    //test deplacement valide
                    if(!i.deplacementValide(draggedX,draggedY, droppedX, droppedY, ennemi)){
                               // System.out.println(" pas normal " +i.deplacementValide(draggedX,draggedY, droppedX, droppedY, ennemi));
                                            deplacement_ok = false;
                            }
                }
            }
            System.out.println(deplacement_ok + " " + ennemi);
            return deplacement_ok;
        }        
        public void move(Pane draggedApp, int XD,int YD,int XF, int YF){
            GridPane.setColumnIndex(draggedApp, XF);
            GridPane.setRowIndex(draggedApp, YF);
            GridPane.setColumnIndex(app, XD);
            GridPane.setRowIndex(app, YD);
        }
        public String inverse_coor(int a){
           String i = "";
            switch (a)
                {

                  case 0:
                    i="8";
                    break;  
                  case 1:
                    i="7";
                    break; 
                  case 2:
                    i="6";
                    break; 
                  case 3:
                    i="5";
                    break; 
                  case 4:
                    i="4";
                    break; 
                  case 5:
                    i="3";
                    break;
                  case 6:
                    i="2";
                    break;
                  case 7:
                    i="1";
                    break;
                }
            return i;
        }
                
        public String nb_to_char(int a){
           String XD = "";
            switch (a)
                {

                  case 0:
                    XD= "A";
                    break;  
                  case 1:
                    XD="B";
                    break; 
                  case 2:
                    XD="C";
                    break; 
                  case 3:
                    XD="D";
                    break; 
                  case 4:
                    XD="E";
                    break; 
                  case 5:
                    XD="F";
                    break;
                  case 6:
                    XD="G";
                    break;
                  case 7:
                    XD="H";
                    break;
                }
            return XD;
        }  
        
        public void handle(DragEvent event) {
                Pane draggedApp = (Pane) event.getGestureSource();
                Pane cible = (Pane) event.getGestureTarget();
                boolean deplacement_ok = true;
                boolean ennemi = false;
                // switch panes:
                int draggedX = GridPane.getColumnIndex(draggedApp);
                int draggedY = GridPane.getRowIndex(draggedApp);
                int droppedX = GridPane.getColumnIndex(app);
                int droppedY = GridPane.getRowIndex(app);
                String XD = nb_to_char(draggedX);
                String XF = nb_to_char(droppedX);
                String YD = inverse_coor(draggedY);
                String YF = inverse_coor(droppedY);
                
                //draggedApp.set
               
                for(Piece i:Piece){
                      
                     if(draggedApp.getChildren().toString().equals(i.getid()))
                     {
                         if(i.gettype().equals("blanc") ){
                              // tour blanc
                              if( tour_blanc == true)
                              {
                                    
                                    deplacement_ok = test_move_valide("noire","blanc",cible,i,draggedX,draggedY, droppedX, droppedY,rock_b_possible);
                                       
                                    
                                     if(deplacement_ok == true){
                                            String phrase =  i.getnom() + " " + XD + " " + YD + " - " + XF + " " + YF ;
                                            olb.add(phrase);
                                            move(draggedApp,draggedX,draggedY,droppedX,droppedY);
                                            tour_blanc=false;
                                            Tour.setText("Tour des noirs !");
                                            ChronoW.stop();
                                            ChronoB.play();
                                     }
                              }
                         }else{
                             // tour noir
                              if( tour_blanc == false)
                                {
                                  
                                    deplacement_ok = test_move_valide("blanc","noire",cible,i,draggedX,draggedY, droppedX, droppedY,rock_n_possible);
                                   
                                    
                                     
                                    if(deplacement_ok == true){
                                        String phrase =  i.getnom() + " " + XD + " " + YD + " - " + XF + " " + YF ;
                                        oln.add(phrase);
                                        move(draggedApp,draggedX,draggedY,droppedX,droppedY);
                                        Tour.setText("Tour des blancs !");
                                        tour_blanc=true;
                                        ChronoB.stop();
                                        ChronoW.play();
                                  }
                                  
                                }
                         }
                     }
                     
                 }
             
                
              }
            });


            } return app;
        }
        @FXML
        private void handleButtonAction(ActionEvent event) {
               Platform.exit();
            }


        @FXML
        private void btnStartClick(ActionEvent event) throws InterruptedException {
            tour_blanc = true;
            charge_pieces();
            Tour.setText("Tour des blancs !");
           
            ChronoW = new Chronometre(tmpW);
            ChronoB = new Chronometre(tmpB);
            ChronoW.setLbl(tmpW);
            ChronoB.setLbl(tmpB);
            ChronoW.play();
            StartGame.setDisable(true);
            /*sleep(1000);
                party();
            }
            
            make_move("a1c5","noire");
            make_move("e1e4","noire");
            make_move("e8d4","noire");*/
        }
        @FXML
        private void btnAbandonClick(ActionEvent event) {
            tour_blanc = true;
            ChronoW.stop();
            ChronoW.reset();
            ChronoB.stop();
            ChronoB.reset();
            vide_grille();
            listCpW.getItems().clear();
            listCpB.getItems().clear();
            tmpW.setText("00:00:000");
            tmpB.setText("00:00:000");
            Tour.setText("");
            StartGame.setDisable(false);
            }

        
        private void vide_grille(){
            grille.getChildren().clear();
        }
        private void restart(){
            tour_blanc = true;
            ChronoW.stop();
            ChronoW.reset();
            ChronoB.stop();
            ChronoB.reset();
            vide_grille();
            listCpW.getItems().clear();
            listCpB.getItems().clear();
            tmpW.setText("00:00:000");
            tmpB.setText("00:00:000");
            Tour.setText("");
            StartGame.setDisable(false);
           }

        ObservableList<String> oln=FXCollections.observableArrayList();
        ObservableList<String> olb=FXCollections.observableArrayList();
        @Override
        public void initialize(URL url, ResourceBundle rb) {
                // TODO
                Piece = new ArrayList<Piece>();
                listCpB.setItems(oln);
                listCpW.setItems(olb);
                
                 //charger le moteur si possible
               //make_move("a1b2","");
                
                //test();
            }     
  
        /*public void Playing_game(String move){
            String whiteMove=move;
            String startPos="";
            uci.move_FromFEN(startPos, whiteMove, true);
            String fenWhite=ChessBoard.moveFromFEN(startPos, whiteMove);
            ChessBoard.show_chessboard();
            
            //black response
            uci.go_Think();
            String blackMove=uci.get_BestMove(true);
             System.out.println("black move "+blackMove);
             uci.move_FromFEN(fenWhite, blackMove, true);
             String fenBlack=ChessBoard.moveFromFEN(fenWhite, blackMove);
             ChessBoard.show_chessboard();
        }*/
        @FXML
        private void party(ActionEvent event) throws InterruptedException{
            
        long i=1000;    
        UCIChess engine1=new UCIChess("./src/Protector_1_6_0/bin/Protector 1.6.0 x64.exe");
        //run engine2
        UCIChess engine2=new UCIChess("./src/stockfish-6-win/Windows/stockfish-6-64.exe");
       //get name of first one
        engine1.get_UciOk(false);
        String nameEngine1=engine1.getEngineName();
        //get name of second's
        engine2.get_UciOk(false);
        String nameEngine2=engine2.getEngineName();
   
        System.out.println(nameEngine1+" is white player.");
        System.out.println(nameEngine2+" is black player.\n");    
        int turn=1;    
        while (turn<=500){
        sleep(i);
        //wait engine1
        engine1.get_ReadyOk(traceMode);
       
            //play white for engine1
            //engine1.go_Think(); //think for best move x seconds
            System.out.println("Wait white thinking...!");
            engine1.go_Think_MoveTime(timeThinking);//think for best move x seconds
            String repw=engine1.get_BestMove(traceMode);  //read response
            if (moves==null){moves=repw;} //just the first move
            else {moves=moves+" "+repw;} //incruise moves list
            System.out.println("\n"+nameEngine1+"=> White play (turn "+turn+") "+repw+"\n");
            fenPos=ChessBoard.moveFromFEN(fenPos, repw);
            ChessBoard.show_wide_chessboard();
            //if black is mate then white say "score mate 1" so test it
            if (engine1.is_opponent_Mated(traceMode)){System.out.println("\nturn("+turn+")"+nameEngine1+" playing WHITE WIN\n");moves=moves+" black is mate";break;}
            //apply moves to all engines
            engine1.move_FromSTART(moves,traceMode); //make move
            engine2.move_FromSTART(moves,traceMode); //make move
            make_move(repw,"blanc");
            sleep(i);
            //wait for engine2
            engine2.get_ReadyOk(traceMode);
       
            //play black for engine2
            //engine2.go_Think(); //think for best move
            System.out.println("Wait black thinking...!");
            engine2.go_Think_MoveTime(timeThinking);//think for best move
            String repb=engine2.get_BestMove(traceMode);  //read response
            moves=moves+" "+repb; //incruise moves list
            System.out.println("\n"+nameEngine2+"=> Black play (turn "+turn+") "+repb+"\n");
            fenPos=ChessBoard.moveFromFEN(fenPos, repb);
            ChessBoard.show_chessboard();
            System.out.println("moves : "+moves+"\n");
            //if white is mate then black say "score mate 1" so test it
            if (engine2.is_opponent_Mated(traceMode)) {System.out.println("\nturn("+turn+")"+nameEngine2+" playing BLACK WIN\n");moves=moves+" white is mate";break;}
            
            //apply moves to all engines
            engine1.move_FromSTART(moves,traceMode); //make move
            engine2.move_FromSTART(moves,traceMode); //make move
            make_move(repb,"noire");
        
   turn++;
   }
        }
        public int char_to_nb(String a){
           int i = 0;
            switch (a)
                {

                  case "a":
                    i=0;
                    break;  
                  case "b":
                    i=1;
                    break; 
                  case "c":
                    i=2;
                    break; 
                  case "d":
                    i=3;
                    break; 
                  case "e":
                    i=4;
                    break; 
                  case "f":
                    i=5;
                    break;
                  case "g":
                    i=6;
                    break;
                  case "h":
                    i=7;
                    break;
                }
            return i;
        }
        public int inverse_coor(String a){
           int i = 0;
            switch (a)
                {

                  case "1":
                    i=7;
                    break;  
                  case "2":
                    i=6;
                    break; 
                  case "3":
                    i=5;
                    break; 
                  case "4":
                    i=4;
                    break; 
                  case "5":
                    i=3;
                    break; 
                  case "6":
                    i=2;
                    break;
                  case "7":
                    i=1;
                    break;
                  case "8":
                    i=0;
                    break;
                }
            return i;
        }
        public void make_move(String move,String couleur){
        final Pane app = new StackPane();
        System.out.println(move );
        
        String[] tokens = new String[6];
        for(int i = 0; i < move.length();i++)
            {
            tokens[i]=(move.substring(i, i+1));
   
            }
        
        int XD = char_to_nb(tokens[0]);
        int YD = inverse_coor(tokens[1]);
        int XF = char_to_nb(tokens[2]);
        int YF = inverse_coor(tokens[3]);
        
       Node a = getNodeByRowColumnIndex(XD,YD); 
       //System.out.println(a);
       System.out.println(move + " " + XD + YD  + XF + YF);
      
      
       move2(a,XF,YF,XD,YD);
    }
   
    public void move2(Node draggedApp, int XD,int YD,int XF, int YF){
            final Pane app = new StackPane();
            
            grille.setColumnIndex(draggedApp, XF);
            grille.setRowIndex(draggedApp, YF);
            grille.setColumnIndex(app, XD);
            grille.setRowIndex(app, YD);
        }
    public Node getNodeByRowColumnIndex(final int row,final int column) {
        Node result = null;
        ObservableList<Node> childrens = grille.getChildren();
        for(Node node : childrens) {
            if(grille.getRowIndex(node) == row && grille.getColumnIndex(node) == column) {
                result = node;
                break;
            }
        }
        return result;
    }
        /*public void test()
        {
              //ask uci infos
            System.out.println("======================TEST UCI COMMAND======================");
           //is uci ok ?
            System.out.println("uciok = "+uci.get_UciOk(false));
            //engine name and author(s)
            System.out.println("Engine Name = "+uci.getEngineName());
            System.out.println("Engine Author(s) = "+uci.get_EngineAuthor());
            System.out.println("==================TEST UCI OPTIONS RETRIEVE=================");
            //number of options in uci engine
            System.out.println("Numbers of options = "+uci.get_Number_Options());
            //list all uci options (names, type, values)
            System.out.format("%-30s %-10s %-20s\n","Name(id)","type","values");
            System.out.println("------------------------------------------------------------");
            for (int i=0;i<uci.get_Number_Options();i++)
            {
                System.out.format("%-30s %-10s %-20s\n",uci.get_Option(i).getId(),uci.get_Option(i).getType(),uci.get_Option(i).getValues() );
            }
            System.out.println("=====================PLAY A SMALL GAME=====================");
            //is engine ready?
            System.out.println("isready = "+uci.get_ReadyOk(false));
            
            //white play e2e4
            System.out.println("White play = e2e4");
            uci.move_FromSTART("e2e4 ",false); 
            System.out.println("-------------------------------------------------------");
            //is engine ready for next move?
            System.out.println("isready = "+uci.get_ReadyOk(false));
            
            //black move (engine play)
            uci.go_Think(); //think for best move
            String rep=uci.get_BestMove(false);  //read response
            System.out.println("---------------info on best move-----------------------");
            System.out.println("Number of infos lines = "+uci.get_Number_SimpleInfo());
            System.out.format("%-50s\n","Info lines");
            System.out.println("-------------------------------------------------------");
            for (int i=0;i<uci.get_Number_SimpleInfo();i++)
            {
                System.out.format("%-50s\n",uci.get_SimpleInfo(i).getInfo());
            }
            System.out.println("-------------------------------------------------------");
            System.out.println("Black play = "+rep); //draw best move
            System.out.println("Black ponder = "+uci.get_Ponder()); //best white next move
            uci.move_FromSTART("e2e4 "+rep,false); //make move
            System.out.println("-------------------------------------------------------");
            
            //is engine ready for next move?
            System.out.println("isready = "+uci.get_ReadyOk(false));
            
            //white play g1f3
            System.out.println("White play = g1f3");
            uci.move_FromSTART("e2e4 "+rep+" g1f3 ",false);
            System.out.println("-------------------------------------------------------");
            //is engine ready for next move?
            System.out.println("isready = "+uci.get_ReadyOk(false));
            
            //black play
            System.out.println("Black thinking 5 seconds wait please....");
            uci.go_Think_MoveTime(5000); //search next move during 5 seconds
            String rep2=uci.get_BestMove(true);  //read best move
            System.out.println("---------------info on best move-----------------------");
            System.out.println("Number of infos lines = "+uci.get_Number_DetailedInfo());
            System.out.format("%-50s\n","Info Details");
            System.out.println("-------------------------------------------------------");
            for (int i=0;i<uci.get_Number_DetailedInfo();i++)
            {
                System.out.format("Step "+i+" Calculate Nodes = %-50s\n",uci.get_DetailedInfo(i).getNodes());
            }
            System.out.println("-------------------------------------------------------");
            System.out.println("Black play = "+rep2); //draw black turn
            System.out.println("Black ponder = "+uci.get_Ponder()); //best white next move
            uci.move_FromSTART("e2e4 "+rep+" g1f3 "+rep2,false); //make move

            System.out.println("-------------------TEST SQUARE--------------------------");
            
            String FEN="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
            ChessBoard.assign_chessboard(FEN);
            
             


             
            //bye bye...
            
            uci.stop_Engine();
        }*/
    


    


   
}
