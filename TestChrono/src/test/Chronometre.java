/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package test;

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
public class Chronometre extends HBox{

    private Label labelMillisecondes;
    private Label labelSecondes;
    private Label labelMinutes;		
    private long timeCounter;
    private Timeline timeline;
    private Label doublePoints1;
    private Label doublePoints2;

    public Chronometre(){
	this(Duration.millis(0));
    }
  
    public Chronometre(Duration duration){		
	timeCounter = (long)duration.toMillis();
	setAlignment(Pos.CENTER);
	doublePoints1 = new Label(":");
	doublePoints2 = new Label(":");		
	labelMillisecondes = new Label();
	labelSecondes = new Label();
	labelMinutes = new Label();
	updateLabels(timeCounter);
	getChildren().addAll(labelMinutes, doublePoints1, labelSecondes, doublePoints2, labelMillisecondes);		
	timeline = new Timeline(new KeyFrame(Duration.millis(1), new EventHandler<ActionEvent>(){
            @Override
            public void handle(ActionEvent arg0){
		updateLabels(timeCounter++);				
        }}));
        timeline.setCycleCount(Timeline.INDEFINITE);
    }
	
    public long getTime(){
        return timeCounter;
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
        labelMillisecondes.setText(sMil);
        labelSecondes.setText(sSec);
        labelMinutes.setText(sMin);
    }

    public void setChronoStyle(String style){
        labelMillisecondes.setStyle(style);
        labelMinutes.setStyle(style);
        labelSecondes.setStyle(style);
        doublePoints1.setStyle(style);
        doublePoints2.setStyle(style);
    }
    
}
