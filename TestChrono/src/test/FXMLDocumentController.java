/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package test;

import java.net.URL;
import java.util.ResourceBundle;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.Initializable;
import javafx.scene.control.Button;
import javafx.scene.control.Label;

/**
 *
 * @author Utilisateur
 */
public class FXMLDocumentController implements Initializable {
    
    @FXML
    private Label lblChrono1;
    @FXML
    private Label lblChrono2;
    @FXML
    private Button btnStart1;
    @FXML
    private Button btnStart2;
    @FXML
    private Button btnStop1;
    @FXML
    private Button btnStop2;
    @FXML
    private Button btnReset1;
    @FXML
    private Button btnReset2;
    
    private Chronometre chrono1;
    private Chronometre chrono2;
      
    @Override
    public void initialize(URL url, ResourceBundle rb) {
          chrono1 = new Chronometre();
          chrono2 = new Chronometre();
    }
    
    @FXML
    private void hbtnStart1(ActionEvent event) {
        chrono1.play();   
        lblChrono1.setText(String.valueOf(chrono1));
        
    }  
    
    @FXML
    private void hbtnStart2(ActionEvent event) {
        lblChrono2.setText(chrono2.toString());
        chrono2.play();        
    } 
    
    @FXML
    private void hbtnStop1(ActionEvent event) {
        chrono1.stop();
    }  
    
    @FXML
    private void hbtnStop2(ActionEvent event) {
        chrono2.stop();
    }
    
    @FXML
    private void hbtnReset1(ActionEvent event) {
        chrono1.reset();
    }  
    
    @FXML
    private void hbtnReset2(ActionEvent event) {
        chrono2.reset();
    }
    
}
