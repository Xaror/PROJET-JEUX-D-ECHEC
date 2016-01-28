/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package test.pkginterface;

import java.net.URL;
import java.util.ResourceBundle;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Label;
import javafx.scene.control.TextArea;
import javafx.stage.Stage;

/**
 *
 * @author Bloody
 */
public class FXMLDocumentController implements Initializable {
    
    @FXML
    private Button closeButton;
    @FXML
    private Button StartGame;
    @FXML
    private Button Abandon;
    @FXML
    private Label tmpW;
    @FXML
    private Chrono chrono;
    @FXML
    private Label tmpB;
    @FXML
    private TextArea listCpW;
     @FXML
    private TextArea listCpB;
    @FXML
    private void handleButtonAction(ActionEvent event) {
       Stage stage = (Stage) closeButton.getScene().getWindow();                
        stage.close();
    }
    
    
    @FXML
    private void btnStartClick(ActionEvent event) {
       
       chrono = new Chrono();
       //var = chrono.play() ;
    }
    
    @Override
    public void initialize(URL url, ResourceBundle rb) {
        // TODO
    }    
 
}
