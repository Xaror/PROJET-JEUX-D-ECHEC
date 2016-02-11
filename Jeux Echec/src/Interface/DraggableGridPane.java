/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package Interface;

import javafx.application.Application;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.image.Image;
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

public class DraggableGridPane extends Application {

  @Override
  public void start(Stage primaryStage) {
    final GridPane root = new GridPane();
    root.setStyle("-fx-background-color: aliceblue");
    root.setGridLinesVisible(true);
    final int appsPerRow = 8;
    for (int i = 0; i < 8; i++) {
      Pane app = createApp(root, i, appsPerRow, false);
      app.getChildren().add(new Text("App " + (i + 1)));
    }
    for (int i = 4; i < 64; i++) {
      createApp(root, i, appsPerRow, true);
    }
    Scene scene = new Scene(root);
    primaryStage.setScene(scene);
    primaryStage.show();
  }

  private Pane createApp(final GridPane root, final int appNumber,
      final int appsPerRow, boolean filler) {
    final Pane app = new StackPane();
    if (filler) {
      app.setStyle("-fx-background-color: transparent;");
    } else {
      app.setStyle("-fx-background-color: white; -fx-border-color:black;");
    }

    final int x = appNumber % appsPerRow;
    final int y = appNumber / appsPerRow;
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
    }
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
      @Override
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

    return app;
  }

  public static void main(String[] args) {
    launch(args);
  }
}