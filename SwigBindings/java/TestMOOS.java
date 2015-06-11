
public class TestMOOS {
  static {
    System.loadLibrary("mooslib.so");
  }

  public static void main(String argv[]) {
    System.out.println(mooslib.MOOSTime());
  }
}

