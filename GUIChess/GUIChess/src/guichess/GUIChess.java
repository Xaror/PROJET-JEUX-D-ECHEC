package guichess;

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
 * @author tondeur-h
 */
public class GUIChess extends Application {

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
        //ouvrir la vue fxml GUI.fxml
        Parent root = FXMLLoader.load(getClass().getResource("GUI.fxml"));
        //creer l'objet scene avec la vue GUI.fxml
        Scene scene = new Scene(root);

        //tuer le moteur de jeu à la fermeture de l'application
        //et demander à l'utilisateur si fermeture OK
        stage.setOnCloseRequest(new EventHandler<WindowEvent>() {

            @Override
            public void handle(WindowEvent event) {
                Alert alertquit=new Alert(Alert.AlertType.CONFIRMATION);
                alertquit.setTitle("Quiiter GUI Chess HT ?");
                alertquit.setContentText("Quitter l'application ?");
                alertquit.showAndWait();
                if (alertquit.getResult()==ButtonType.OK){
                    try {
                        File f=new File(read_Engine());
                        System.out.println("run : "+"taskkill /IM "+f.getName()+" /T /F");
                        Runtime.getRuntime().exec("taskkill /IM "+f.getName()+" /T /F");
                    } catch (IOException ex) {
                        Logger.getLogger(GUIChess.class.getName()).log(Level.SEVERE, null, ex);
                    }
                }

            }
        });



        //affecter la scene au la fenêtre principale
        stage.setScene(scene);
        stage.setTitle("GUI Chess HT - Tondeur Hervé Version 1.0"); //titre de la fenêtre
        stage.setResizable(false); //ne peut pas être agrandie ou réduite
        stage.centerOnScreen(); //centrer sur l'écran au démarrage de l'appli
        stage.show(); //afficher la fenêtre
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        launch(args);
    }

}
