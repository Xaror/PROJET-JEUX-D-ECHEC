/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package test;

import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;
/**
 *
 * @author Utilisateur
 */
public class Test extends Application{

    public static void main(String[] args){
	launch(args);
    }
    
    private Chronometre chrono;
    private Chronometre chrono2;

    @Override
    public void start(final Stage stage) throws Exception{
        VBox vbox = new VBox(10.0);
        vbox.setAlignment(Pos.CENTER);
	String style = "-fx-font: 50pt \"Arial\";-fx-text-fill: blue;";
		
        chrono = new Chronometre();
        chrono.setChronoStyle(style);		
		
        chrono2 = new Chronometre();
        chrono2.setChronoStyle(style);
            
        Button start = new Button("Start");
        start.setOnAction(new EventHandler<ActionEvent>(){			
            @Override
            public void handle(ActionEvent arg0){
                chrono.play();
                chrono2.play();
            }
        });
		
        Button stop = new Button("Stop");
        stop.setOnAction(new EventHandler<ActionEvent>(){			
            @Override
            public void handle(ActionEvent arg0){
                chrono.stop();
                chrono2.stop();
            }
        });

        Button reset = new Button("Reset");
        reset.setOnAction(new EventHandler<ActionEvent>(){			
            @Override
            public void handle(ActionEvent arg0){
                chrono.reset();
                chrono2.reset();
            }
        });
		
        vbox.getChildren().addAll(chrono, start, stop, reset, chrono2);
        Scene scene = new Scene(vbox, 500, 500);
        stage.setScene(scene);
        stage.show();
    }
    
}