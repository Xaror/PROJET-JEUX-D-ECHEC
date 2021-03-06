/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package JeuxEchecs;

import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.util.Duration;
/**
 *
 * @author Utilisateur
 */
public class Chronometre{

    private Label Lbl;
    private long timeCounter;
    private Timeline timeline = new Timeline();
    private long chrono = 0; 
    private long temps = 0 ;
    private long temps_total = 0;
    private boolean first = true;
    
   
    public Chronometre(Label lb){
        Lbl=lb;
    }
    	
    public long getTime(){
        return timeCounter;
    }
    public void setLbl(Label lb){
        Lbl = lb;
    }
	
    public Duration getDuration(){
        return Duration.millis(timeCounter);
    }
	
    public void setTime(long time){
        this.timeCounter = time;
        updateLabels(timeCounter);
    }

    public void setDuration(Duration time){
        this.timeCounter = (long)time.toMillis();
        updateLabels(timeCounter);
    }

    public void play(){
       
        timeline = new Timeline(new KeyFrame(Duration.millis(1), new EventHandler<ActionEvent>(){
            @Override
            public void handle(ActionEvent arg0){
		updateLabels(timeCounter++);				
        }}));
        timeline.setCycleCount(Timeline.INDEFINITE);
        timeline.play();
    }

    public void stop(){
        timeline.stop();
        
    }
	
    public void reset(){
        stop();
        timeCounter = 0L;
    }

    protected void updateLabels(long timeCounter){
        long millisecondes = timeCounter%1000;
        long secondes = (timeCounter/1000)%60;
        long minutes = (timeCounter/60000)%60;
        String sMil = millisecondes<10?("00"+millisecondes):millisecondes<100?("0"+millisecondes):(""+millisecondes);
        String sSec = secondes<10?("0"+secondes):(""+secondes);
        String sMin = minutes<10?("0"+minutes):(""+minutes);
        Lbl.setText(sMin + ":" + sSec + ":" + sMil);
        
    }
    
    
        
        
    

    
    
}
