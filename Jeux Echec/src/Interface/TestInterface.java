/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Interface;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import javafx.event.EventHandler;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import javafx.stage.Stage;
import javafx.stage.WindowEvent;

/**
 *
 * @author Bloody
 */
public class TestInterface extends Application {
    
        /**
     * Lire le nom du moteur d'echec
     * @return
     */
    private String read_Engine(){
        Properties p=new Properties(); //ouvrir le fichier propriété
        try {
            p.loadFromXML(new FileInputStream("engine.xml"));
        } catch (FileNotFoundException ex) {return null;} catch (IOException ex) {return null;}
        return p.getProperty("ENGINE");
    }    
    
    @Override
    public void start(Stage stage) throws Exception {
        Parent root = FXMLLoader.load(getClass().getResource("Interface.fxml"));
        
        Scene scene = new Scene(root);
        
        //tuer le moteur de jeu à la fermeture de l'application
        //et demander à l'utilisateur si fermeture OK
        stage.setOnCloseRequest(new EventHandler<WindowEvent>() {

            @Override
            public void handle(WindowEvent event) {
                Alert alertquit=new Alert(Alert.AlertType.CONFIRMATION);
                alertquit.setTitle("Quitter GUI Chess HT ?");
                alertquit.setContentText("Quitter l'application ?");
                alertquit.showAndWait();
                if (alertquit.getResult()==ButtonType.OK){
                    try {
                        File f=new File(read_Engine());
                        System.out.println("run : "+"taskkill /IM "+f.getName()+" /T /F");
                        Runtime.getRuntime().exec("taskkill /IM "+f.getName()+" /T /F");
                    } catch (IOException ex) {
                        Logger.getLogger(TestInterface.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }

            }
        });        
        
        stage.setScene(scene);
        
        //mettre un titre a l'application
        stage.setTitle("Jouons aux échecs");
        //rendre la fenêtre non redimensionnable
        stage.setResizable(false);

        stage.show();
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }
    
}
