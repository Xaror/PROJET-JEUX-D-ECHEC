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
    private TextArea listCpW;
    @FXML
    private TextArea listCpB;
    @FXML
    private GridPane grille;
    
    boolean isEngineUP;
     
       //charger tour 
            ImageView TB1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TB.gif")));     
            ImageView TN1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TN.gif")));
            ImageView TB2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TB.gif")));
            ImageView TN2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/TN.gif")));
        
        //charger cav     
            ImageView CB1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CB.gif")));
            ImageView CN1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CN.gif")));
            ImageView CB2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CB.gif")));
            ImageView CN2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/CN.gif")));
            
        //charger fou   
            ImageView FB1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FB.gif")));
            ImageView FN1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FN.gif")));
            ImageView FB2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FB.gif")));
            ImageView FN2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/FN.gif")));
            
         //charger dame     
            ImageView DB = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/DB.gif")));
            ImageView DN = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/DN.gif")));
            
        //charger roi blanc    
            ImageView RB = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/RB.gif")));
            ImageView RN = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/RN.gif")));
           
          
        //charger pion     
            ImageView PB1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN1 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            ImageView PB2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN2 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            ImageView PB3 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN3 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            ImageView PB4 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN4 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            ImageView PB5 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN5 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            ImageView PB6 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN6 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            ImageView PB7 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN7 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            ImageView PB8 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PB.gif")));
            ImageView PN8 = new ImageView(new Image(Control_Interface.class.getResourceAsStream("font/PN.gif")));
            
           
        
            
        
        private void charge_pieces(){
                       
           /* grille.add(TN1,0,0);
            grille.add(CN1,1,0);
            grille.add(FN1,2,0);
            grille.add(RN,3,0);
            grille.add(DN,4,0);
            grille.add(FN2,5,0);
            grille.add(CN2,6,0);
            grille.add(TN2,7,0);
            grille.add(PN1,0,1);
            grille.add(PN2,1,1);
            grille.add(PN3,2,1);
            grille.add(PN4,3,1);
            grille.add(PN5,4,1);
            grille.add(PN6,5,1);
            grille.add(PN7,6,1);
            grille.add(PN8,7,1);
            
            grille.add(TB1,0,7);
            grille.add(CB1,1,7);
            grille.add(FB1,2,7);
            grille.add(RB,3,7);
            grille.add(DB,4,7);
            grille.add(FB2,5,7);
            grille.add(CB2,6,7);
            grille.add(TB2,7,7);
            grille.add(PB1,0,6);
            grille.add(PB2,1,6);
            grille.add(PB3,2,6);
            grille.add(PB4,3,6);
            grille.add(PB5,4,6);
            grille.add(PB6,5,6);
            grille.add(PB7,6,6);
            grille.add(PB8,7,6);
            
            */
            
            grille.setGridLinesVisible(true);
            final int appsPerRow = 8;
            
                Pane app = createApp(grille, 0, appsPerRow, true);
                 app.getChildren().add(TN1);
                 app = createApp(grille, 1, appsPerRow, true);
                 app.getChildren().add(CN1);
                 app = createApp(grille, 2, appsPerRow, true);
                 app.getChildren().add(FN1);
                 app = createApp(grille, 3, appsPerRow, true);
                 app.getChildren().add(RN);
                 app = createApp(grille, 4, appsPerRow, true);
                 app.getChildren().add(DN);
                 app = createApp(grille, 5, appsPerRow, true);
                 app.getChildren().add(FN2);
                 app = createApp(grille, 6, appsPerRow, true);
                 app.getChildren().add(CN2);
                 app = createApp(grille, 7, appsPerRow, true);
                 app.getChildren().add(TN2);
                 
                 app = createApp(grille, 8, appsPerRow, true);
                 app.getChildren().add(PN1);
                 app = createApp(grille, 9, appsPerRow, true);
                 app.getChildren().add(PN2);
                 app = createApp(grille, 10, appsPerRow, true);
                 app.getChildren().add(PN3);
                 app = createApp(grille, 11, appsPerRow, true);
                 app.getChildren().add(PN4);
                 app = createApp(grille, 12, appsPerRow, true);
                 app.getChildren().add(PN5);
                 app = createApp(grille, 13, appsPerRow, true);
                 app.getChildren().add(PN6);
                 app = createApp(grille, 14, appsPerRow, true);
                 app.getChildren().add(PN7);
                 app = createApp(grille, 15, appsPerRow, true);
                 app.getChildren().add(PN8);
                 
                 
                 app = createApp(grille, 56, appsPerRow, false);
                 app.getChildren().add(TB1);
                 app = createApp(grille, 57, appsPerRow, false);
                 app.getChildren().add(CB1);
                 app = createApp(grille, 58, appsPerRow, false);
                 app.getChildren().add(FB1);
                 app = createApp(grille, 59, appsPerRow, false);
                 app.getChildren().add(RB);
                 app = createApp(grille, 60, appsPerRow, false);
                 app.getChildren().add(DB);
                 app = createApp(grille,61, appsPerRow, false);
                 app.getChildren().add(FB2);
                 app = createApp(grille, 62, appsPerRow, false);
                 app.getChildren().add(CB2);
                 app = createApp(grille, 63, appsPerRow, false);
                 app.getChildren().add(TB2);
                 
                 app = createApp(grille, 55, appsPerRow, false);
                 app.getChildren().add(PB1);
                 app = createApp(grille, 54, appsPerRow, false);
                 app.getChildren().add(PB2);
                 app = createApp(grille, 53, appsPerRow, false);
                 app.getChildren().add(PB3);
                 app = createApp(grille, 52, appsPerRow, false);
                 app.getChildren().add(PB4);
                 app = createApp(grille, 51, appsPerRow, false);
                 app.getChildren().add(PB5);
                 app = createApp(grille, 50, appsPerRow, false);
                 app.getChildren().add(PB6);
                 app = createApp(grille, 49, appsPerRow, false);
                 app.getChildren().add(PB7);
                 app = createApp(grille, 48, appsPerRow, false);
                 app.getChildren().add(PB8);
                
    /*for (int i = 0; i < 4; i++) {
      Pane app = createApp(grille, i, appsPerRow, false);
      app.getChildren().add(new Text("App " + (i + 1)));
    }*/
            for (int i = 1; i < 48; i++) {
                createApp(grille, i, appsPerRow, false);
            }
        }
       
        
        
        
        private Pane createApp(final GridPane root, final int appNumber,final int appsPerRow, boolean filler) {
            final Pane app = new StackPane();
            

            final int x = appNumber % appsPerRow;
            final int y = appNumber / appsPerRow;
            System.out.println("x " + x);
            System.out.println("y " + y);
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
                // switch panes:
                int draggedX = GridPane.getColumnIndex(draggedApp);
                int draggedY = GridPane.getRowIndex(draggedApp);
                int droppedX = GridPane.getColumnIndex(app);
                int droppedY = GridPane.getRowIndex(app);
                GridPane.setColumnIndex(draggedApp, droppedX);
                GridPane.setRowIndex(draggedApp, droppedY);
                GridPane.setColumnIndex(app, draggedX);
                GridPane.setRowIndex(app, draggedY);
              }
            });


            } return app;
                }
            @FXML
            private void handleButtonAction(ActionEvent event) {
               Stage stage = (Stage) closeButton.getScene().getWindow();                
                stage.close();
            }


            @FXML
            private void btnStartClick(ActionEvent event) {

              // chrono = new Chrono();
               //var = chrono.play() ;
            }







            @Override
            public void initialize(URL url, ResourceBundle rb) {
                // TODO
                charge_pieces();
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