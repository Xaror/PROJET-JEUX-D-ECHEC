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

        public void handle(DragEvent event) {
                Pane draggedApp = (Pane) event.getGestureSource();
                Pane cible = (Pane) event.getGestureTarget();
                boolean deplacement_ok = true;
                // switch panes:
                int draggedX = GridPane.getColumnIndex(draggedApp);
                int draggedY = GridPane.getRowIndex(draggedApp);
                int droppedX = GridPane.getColumnIndex(app);
                int droppedY = GridPane.getRowIndex(app);
                String XD = "";
                String XF = "";
                switch (draggedX)
                {

                  case 0:
                    XD="A";
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
                switch (droppedX)
                {

                  case 0:
                    XF="A";
                    break;  
                  case 1:
                    XF="B";
                    break; 
                  case 2:
                    XF="C";
                    break; 
                  case 3:
                    XF="D";
                    break; 
                  case 4:
                    XF="E";
                    break; 
                  case 5:
                    XF="F";
                    break;
                  case 6:
                    XF="G";
                    break;
                  case 7:
                    XF="H";
                    break;
                }
                //draggedApp.set
                System.out.println(" tour debut" + tour_blanc);
                for(Piece i:Piece){
                      
                     if(draggedApp.getChildren().toString().equals(i.getid()))
                     {
                         if(i.gettype().equals("blanc") ){
                              // tour blanc
                              if( tour_blanc == true)
                              {
                                    System.out.println(" move blanc " );
                                    for(Piece j:Piece){
                                    if(cible.getChildren().toString().equals(j.getid())){
                                        if(j.gettype().equals("noire")){
                                            cible.getChildren().clear();
                                            System.out.println("ok");
                                            
                                        }
                                        if(j.gettype().equals("blanc")){
                                            if((j.getnom().equals("Tour") && i.getnom().equals("Roi")) || (i.getnom().equals("Tour") && j.getnom().equals("Roi")) ){
                                                if(rock_b_possible == true)
                                                {
                                                    
                                                    System.out.println(j.getnom() + " " + i.getnom());
                                                    rock_b_possible = false;
                                                    System.out.println(rock_b_possible);
                                                }else{
                                                    deplacement_ok = false;
                                                }
                                             }else deplacement_ok = false;
                                             }
                                         }
                                     }
                                    
                                     if(deplacement_ok == true){
                                            String phrase =  i.getnom() + " " + XD + " " + draggedY + " - " + XF + " " + droppedY ;
                                            olb.add(phrase);
                                            GridPane.setColumnIndex(draggedApp, droppedX);
                                            GridPane.setRowIndex(draggedApp, droppedY);
                                            GridPane.setColumnIndex(app, draggedX);
                                            GridPane.setRowIndex(app, draggedY);
                                            tour_blanc=false;
                                            Tour.setText("Tour noir !");
                                            ChronoW.stop();
                                            ChronoB.play();
                                     }
                              }
                         }else{
                             // tour noir
                              if( tour_blanc == false)
                                {
                                  
                                  for(Piece j:Piece){
                                    if(cible.getChildren().toString().equals(j.getid())){
                                        if(j.gettype().equals("blanc")){
                                            cible.getChildren().clear();
                                            System.out.println("ok");
                                        }
                                        if(j.gettype().equals("noire")){
                                            if(j.getnom().equals("Tour") && i.getnom().equals("Roi") || i.getnom().equals("Tour") && j.getnom().equals("Roi") ){
                                                if(rock_n_possible == true)
                                                {
                                                    System.out.println(rock_n_possible);
                                                    rock_n_possible = false;
                                                    System.out.println(rock_n_possible);
                                                }else{
                                                    deplacement_ok = false;
                                                }
                                             }else deplacement_ok = false;
                                        }
                                    }
                                  }
                                  if(deplacement_ok == true){
                                        String phrase = i.getnom() + " " + XD + " " + draggedY + " - " + XF + " " + droppedY ;
                                        oln.add(phrase);
                                        System.out.println(" move noir " );
                                        GridPane.setColumnIndex(draggedApp, droppedX);
                                        GridPane.setRowIndex(draggedApp, droppedY);
                                        GridPane.setColumnIndex(app, draggedX);
                                        GridPane.setRowIndex(app, draggedY);
                                        tour_blanc=true;
                                        System.out.println(" timer noir " + ChronoW.getTime());
                                        System.out.println(ChronoB.getTime());
                                        ChronoB.stop();
                                        ChronoW.play();
                                  }
                                  
                                }
                         }
                     }
                     
                 }System.out.println(" tour fin " + tour_blanc);
             
                
              }
            });


            } return app;
        }
        @FXML
        private void handleButtonAction(ActionEvent event) {
               Platform.exit();
            }


        @FXML
        private void btnStartClick(ActionEvent event) {
            tour_blanc = true;
            charge_pieces();
            Tour.setText("Tour blanc !");
           
            ChronoW = new Chronometre(tmpW);
            ChronoB = new Chronometre(tmpB);
            ChronoW.setLbl(tmpW);
            ChronoB.setLbl(tmpB);
            ChronoW.play();
            StartGame.setDisable(true);
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
            StartGame.setDisable(false);
            }

        
        private void vide_grille(){
            grille.getChildren().clear();
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
               /* btnMoteur.setText("MOTEUR : Aucun");
                String engine=read_Engine();
                //si aucun moteur alors seule solution le charger
                if (!isEngineUP){
                    StartGame.setDisable(true);
                    Abandon.setDisable(true);
                }
                else
                {
                     startEngine(engine);
                }*/
            }     
  
   

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
            Logger.getLogger(Control_Interface.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(Control_Interface.class.getName()).log(Level.SEVERE, null, ex);
        }
    }


    /**
     * lance le moteur d'échec...
     * @param path
     */
    /**
    private void startEngine(String path){
        uci=new UCIChess(path);
        uci.get_UciOk(DEBUG);
        btnMoteur.setText("Moteur : "+uci.getEngineName());
        init_board();
        affiche_Board();
        uci.send_uciNewGame();
    }
**/

   
}