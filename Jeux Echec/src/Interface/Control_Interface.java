/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Interface;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.geometry.Pos;
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
    private Label tmpW;
    @FXML
    private Label tmpB;
    @FXML
    private TextArea listCpW;
    @FXML
    private TextArea listCpB;
    @FXML
    private GridPane grille;
    
     
       
           
        
        
        private void charge_pieces(){
            
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
            
        
        
        

       
            for(int i=0;i<=7;i++)
            {
                for(int j=0;j<=7;j++)
                {
                    StackPane ij = new StackPane();
                }
            }
           
            grille.add(TN1,0,0);
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
    
    
    public void drag_drop()
    {
        StackPane sourcePane = new StackPane(new Text("Source"));    
        StackPane targetPane = new StackPane(new Text("Target"));        

        sourcePane.setOnDragDetected(new EventHandler<MouseEvent>() {
        @Override
        public void handle(MouseEvent event) {
            Dragboard db = sourcePane.startDragAndDrop(TransferMode.ANY);
            ClipboardContent content = new ClipboardContent();
            content.putString("Hello!");
            db.setContent(content);
            System.out.println("Drag detected");
            event.consume();
        }
    });


        targetPane.setOnDragOver(new EventHandler<DragEvent>() {
            @Override
            public void handle(DragEvent event) {
                event.acceptTransferModes(TransferMode.ANY);
                System.out.println("Drag over detected");
                event.consume();
            }
        });

        targetPane.setOnDragDropped(new EventHandler<DragEvent>() {
            @Override
            public void handle(DragEvent event) {
                event.acceptTransferModes(TransferMode.ANY);
                System.out.println("Drop detected");
                event.consume();
            }
        });

        grille.add(sourcePane, 5, 1);
        grille.add(targetPane, 0, 1);
    }
    
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
        charge_pieces();
        drag_drop();
        
    }        
}
   
